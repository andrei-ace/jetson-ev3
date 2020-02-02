'''
Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
'''

"""Voice Command Preprocessing Utilities

Provides functions to augment the datasets with background noises, time shift, time stretch,
pitch shift and dynamic range compression (DRC). Also provides function to extract features
(Mel Spectrogram, delta of order 1 and delta of order 2) from the datasets which are used to
train the network.
"""

from concurrent.futures import ProcessPoolExecutor
from functools import partial
from glob import glob
import h5py
import librosa
import logging
from multiprocessing import cpu_count
import numpy as np
import os
from os.path import join
import shutil
import soundfile
import subprocess
import time
from tqdm import tqdm


class DataConfig:
    kSampleRate = 16000
    kFftLength = 400
    kHopLength = 200
    kNumMels = 40
    kWindowLength = None
    kNumFeatures = 3
    kNumNoiseAugmentations = None


def read_audio_file(file_path, target_sample_rate, duration=None, samples=None):
    """Read audio samples from a file. If duration/samples argument is specified audio
    is padded or clipped to match the value.
    """
    try:
        y, sample_rate = soundfile.read(file_path)
        y = librosa.to_mono(y.T)
        y = librosa.resample(y, sample_rate, target_sample_rate)
    except:
        logging.error('Failed to read audio from "{}"'.format(file_path))
        raise

    if duration or samples:
        if duration:
            total_samples = int(target_sample_rate * duration)
        elif samples:
            total_samples = samples
        if len(y) < total_samples:
            # Pad audio files smaller than duration with silence
            pad = total_samples - len(y)
            y = np.concatenate((y, np.zeros(pad)))
        elif len(y) > total_samples:
            # Clip audio files longer than duration
            y = y[:total_samples]
    return y


def create_dir(path):
    """Clears a directory and creates it on disk.

    Args:
        path: The path on disk where the directory should be located.
    """
    os.makedirs(path, exist_ok=True)


def remove_create_dir(path):
    """Clears a directory and creates it on disk.

    Args:
        path: The path on disk where the directory should be located.
    """
    if os.path.exists(path):
        shutil.rmtree(path)
    os.makedirs(path, exist_ok=True)


def mel_spectrogram(y, sample_rate, n_mels, n_fft, hop_length):
    """Compute Mel-Frequency Spectrogram for a time-series data.
    Reference: https://en.wikipedia.org/wiki/Mel-frequency_cepstrum

    Args:
        y: Numpy array of audio samples.
        sample_rate: Sampling rate of audio.
        n_mels: Number of mels.
        n_fft: Length of FFT window.
        hop_length: Stride between consecutive FFT windows.
    Returns:
        2-d array of Mel Spectrogram values in dB with shape (n_ceps, t).
    """
    spectrogram = librosa.core.stft(y, n_fft=n_fft, hop_length=hop_length, center=False)
    mel_filter = librosa.filters.mel(sr=sample_rate, n_fft=n_fft, n_mels=n_mels, fmin=0.)
    mel_spectrogram = librosa.power_to_db(np.dot(mel_filter, np.abs(spectrogram)**2.0), ref=np.max)
    return mel_spectrogram


#############################
#     Data Augmentation     #
#############################
def _save_orig(out_dir, y, sr, index, label):
    """Save the original data as wav file"""
    orig_filename = '{}-{}-orig.wav'.format(label, index)
    librosa.output.write_wav(join(out_dir, orig_filename), y, sr)
    return (orig_filename, label, y.shape[0])


def _time_shift(out_dir, index, i, y, sr, label):
    """Time shift the data and save as wav file"""
    ts_filename = '{}-{}-ts{}.wav'.format(label, index, i)
    librosa.output.write_wav(join(out_dir, ts_filename), y, sr)
    return (ts_filename, label, y.shape[0])


def _time_stretch(out_dir, index, i, y, sr, rate, label):
    """Time stretch the data and save as wav file"""
    y_stretch = librosa.effects.time_stretch(y, rate)
    ts_filename = '{}-{}-t{}.wav'.format(label, index, i)
    librosa.output.write_wav(join(out_dir, ts_filename), y_stretch, sr)
    return (ts_filename, label, y_stretch.shape[0])


def _pitch_shift(out_dir, index, i, y, sr, steps, label):
    """Shift the pitch of the data and save as wav file"""
    y_shift = librosa.effects.pitch_shift(y, sr, n_steps=steps)
    ps_filename = '{}-{}-p{}.wav'.format(label, index, i)
    librosa.output.write_wav(join(out_dir, ps_filename), y_shift, sr)
    return (ps_filename, label, y_shift.shape[0])


def _drc(out_dir, index, i, infile, preset, label):
    """Apply a dynamic range compression on the data and save as wav file"""
    drc_filename = '{}-{}-drc{}.wav'.format(label, index, i)
    arguments = ['sox', '-G', infile, join(out_dir, drc_filename), '-q', 'compand']
    arguments.extend(preset)

    subprocess.check_call(arguments)
    y_drc = read_audio_file(join(out_dir, drc_filename), DataConfig.kSampleRate)
    return (drc_filename, label, y_drc.shape[0])


def _save_chunk(out_dir, index, i, y, sr, label):
    """Save the data to a wav file."""
    chunk_filename = '{}-{}-n{}.wav'.format(label, index, i)
    librosa.output.write_wav(join(out_dir, chunk_filename), y, sr)
    return (chunk_filename, label, y.shape[0])


def _bg_noise(out_dir, index, i, y, sr, bgn, gain, label):
    """Augment background noises to the data and save to a wav file."""
    start = np.random.randint(0, len(bgn))
    noise = np.tile(bgn, int(np.ceil((len(y) + start) / len(bgn))))[start:start + len(y)]

    y = librosa.util.normalize(y)
    noise = librosa.util.normalize(noise)
    w = np.random.uniform(gain['min'], gain['max'], size=None)
    y_noise = ((1.0 - w) * y + w * noise)
    bgn_filename = '{}-{}-bgn{}.wav'.format(label, index, i)
    librosa.output.write_wav(join(out_dir, bgn_filename), y_noise, sr)
    return (bgn_filename, label, y_noise.shape[0])


def write_metadata(metadata, out_dir, sr=16000):
    """Log the metadata of the augmented files"""
    with open(join(out_dir, 'log-augment.txt'), 'w', encoding='utf-8') as f:
        f.write('|'.join(['filename', 'label', 'number of samples']) + '\n')
        for m in metadata:
            f.write('|'.join([str(x) for x in m]) + '\n')
    frames = sum([m[2] for m in metadata])
    hours = frames / (sr * 3600)
    logging.debug('Generated {} audio samples with total {:.2f} hours'.format(len(metadata), hours))


def augment_data(in_dir,
                 out_dir,
                 keyword_length,
                 augment_noise=True,
                 noise_dir=None,
                 noise_gain=None,
                 num_workers=1,
                 tqdm=lambda x: x,
                 valid_mode=False):
    """Augment the audio files with time shift, time stretch, pitch shift, dynamic range
    compression (DRC) and background noises to create an augmented dataset. The generated
    dataset will be approximately 34x larger than the input dataset
    """
    logging.debug('==================================================================')
    logging.debug('||                     A U G M E N T I N G                      ||')
    logging.debug('==================================================================')
    logging.debug('Saving augmented dataset to {}'.format(out_dir))

    sample_rate = DataConfig.kSampleRate

    kTimeShiftSteps = [0, 400]    #TODO: Decide with shift length and audio window length
    kTimeStretchRate = [0.95, 1.05, 1.10, 1.07, 1.15]    #TODO: Decide with time_stretch_rate
    kPitchShiftSteps = [-2, -1, 1, 2]
    kPitchShiftSteps2 = [-3.5, -2.5, 2.5, 3.5]
    kDrcPresets = [
    # # radio
    #     ['0.01,1', '-90,-90,-70,-70,-60,-20,0,0', '-5'],
    # # film standard
    #     ['0.1,0.3', '-90,-90,-70,-64,-43,-37,-31,-31,-21,-21,0,-20', '0', '0', '0.1'],
    # # music standard
    #     ['0.1,0.3', '-90,-90,-70,-58,-55,-43,-31,-31,-21,-21,0,-20', '0', '0', '0.1'],
    # # speech
    #     ['0.1,0.3', '-90,-90,-70,-55,-50,-35,-31,-31,-21,-21,0,-20', '0', '0', '0.1']
    ]
    bg_noises = []
    if augment_noise:
        if noise_dir is not None:
            for bgn_filename in glob(join(noise_dir, '*.wav')):
                bg_y = read_audio_file(bgn_filename, sample_rate)
                bg_noises.append(bg_y)
        if noise_gain is None:
            noise_gain = {'min': 0.1, 'max': 0.4}

    remove_create_dir(out_dir)
    wav_out_dir = join(out_dir, 'wav')
    remove_create_dir(wav_out_dir)
    remove_create_dir(join(out_dir, 'jams'))

    executor = ProcessPoolExecutor(max_workers=num_workers)
    futures = []
    index = 0

    # Save noise samples as unknownkeywords class
    for bg_noise in tqdm(bg_noises, desc='Noises    ', dynamic_ncols=True):
        index += 1
        label = 'unknownkeywords-noise'
        for i, start in enumerate(range(0, len(bg_noise) - keyword_length, keyword_length // 2)):
            futures.append(
                executor.submit(
                    partial(_save_chunk, wav_out_dir, index, i,
                            bg_noise[start:start + keyword_length], sample_rate, label)))

    # Skip augmenting unknownkeywords in validation mode.
    skip_class = ['unknownkeywords'] if valid_mode else []

    for wavfile in tqdm(
            sorted(glob(join(in_dir, '*/*.wav'))), desc='Loading   ', dynamic_ncols=True):
        index += 1
        class_name = wavfile.split('/')[-2]
        label = class_name + '-' + wavfile.split('/')[-1].split()[0].split('.')[0]

        # Load the waveform 'y' with sample rate
        y = read_audio_file(wavfile, sample_rate, duration=None)

        # Save original waveform
        futures.append(
            executor.submit(partial(_save_orig, wav_out_dir, y, sample_rate, index, label)))

        if class_name in skip_class:
            continue

        ts_index = 1
        ps_index = 1
        bgn_index = 1

        # Time Shift waveform
        for i, shift in enumerate(kTimeShiftSteps):
            ys = y[shift:]
            if shift != 0:
                futures.append(
                    executor.submit(
                        partial(_time_shift, wav_out_dir, index, i, y[shift:], sample_rate, label)))

            # Time Stretch the waveform
            for j, rate in enumerate(kTimeStretchRate):
                futures.append(
                    executor.submit(
                        partial(_time_stretch, wav_out_dir, index, ts_index, ys, sample_rate, rate,
                                label)))
                ts_index += 1

            # Pitch Shift the waveform
            for steps in (kPitchShiftSteps + kPitchShiftSteps2):
                futures.append(
                    executor.submit(
                        partial(_pitch_shift, wav_out_dir, index, ps_index, ys, sample_rate, steps,
                                label)))
                ps_index += 1

            if augment_noise:
                # Background Noise augmentation - Augment random noise
                if DataConfig.kNumNoiseAugmentations:
                    bgn_choices = np.random.choice(
                        len(bg_noises), DataConfig.kNumNoiseAugmentations, replace=False)
                else:
                    bgn_choices = range(len(bg_noises))
                for j in bgn_choices:
                    futures.append(
                        executor.submit(
                            partial(_bg_noise, wav_out_dir, index, bgn_index, ys, sample_rate,
                                    bg_noises[j], noise_gain, label)))
                    bgn_index += 1

        # Dynamic Range Compression (DRC)
        for i, preset in enumerate(kDrcPresets):
            futures.append(
                executor.submit(partial(_drc, wav_out_dir, index, i, wavfile, preset, label)))

    # Collect results of each augmentation and save the metadata
    metadata = [future.result() for future in tqdm(futures, desc='Augmenting', dynamic_ncols=True)]
    write_metadata(metadata, out_dir, sample_rate)
    logging.debug('Saved augmented dataset to {}'.format(out_dir))
    logging.debug('Saved metadata of augmented dataset in {}'.format(out_dir))


def extract_features(data_dir, feature_path, keyword_id_map):
    """Extract mel spectrogram, deltas of order 1 and order 2 features from the audio files.
    """
    logging.debug('==================================================================')
    logging.debug('||             E X T R A C T I N G   F E A T U R E S            ||')
    logging.debug('==================================================================')

    num_classes = len(keyword_id_map)
    wav_path = join(data_dir, 'wav')
    t_start = time.time()
    failed_count = 0
    features_all = []
    labels_all = []

    num_samples = DataConfig.kHopLength * DataConfig.kWindowLength + \
                  (DataConfig.kFftLength - DataConfig.kHopLength)

    audio_files = [filename for filename in os.listdir(wav_path) if filename.endswith('.wav')]
    audio_files = sorted(audio_files)
    logging.debug('Total number of audio files to extract: {}'.format(len(audio_files)))

    for filename in tqdm(audio_files, desc='Extracting', dynamic_ncols=True):
        wav_filepath = join(wav_path, filename)
        classIdx = int(keyword_id_map[filename.split('-')[0]])

        audio = read_audio_file(wav_filepath, DataConfig.kSampleRate, samples=num_samples)
        if audio.shape[0] == 0:
            logging.warn('File {} is corrupted.'.format(wav_filepath))
        else:
            feature_mels = mel_spectrogram(audio, DataConfig.kSampleRate, DataConfig.kNumMels,
                                           DataConfig.kFftLength, DataConfig.kHopLength)
            feature_mels = feature_mels[:, :DataConfig.kWindowLength] / 80. + 0.5

            feature_delta1 = librosa.feature.delta(feature_mels, order=1, width=5)
            feature_delta2 = librosa.feature.delta(feature_mels, order=2, width=5)
            features = np.stack((feature_mels.T, feature_delta1.T, feature_delta2.T), axis=-1)

            y = np.zeros(num_classes)
            if len(features) != DataConfig.kWindowLength:
                failed_count = failed_count + 1
                logging.error('Failed to extract features for {}'.format(filename))
            else:
                y[classIdx] = 1
                features_all.append(features)
                labels_all.append(y)

    features_all = np.stack(features_all, axis=0)

    with h5py.File(feature_path, 'w') as hf:
        hf.create_dataset('input', data=features_all)
        hf.create_dataset('labels', data=labels_all)

    logging.debug('Time taken for extracting features: {}'.format(time.time() - t_start))
    logging.debug('Extracted features to {}'.format(feature_path))


def preprocess_data(dataset_path,
                    output_path,
                    augment_noise,
                    noise_profiles,
                    keyword_id_map,
                    keyword_duration,
                    noise_gain,
                    cpu_usage=None):
    """Augment the datasets and extract features from it."""
    keyword_length = int(keyword_duration * DataConfig.kSampleRate / 1000)
    DataConfig.kWindowLength = int(
        np.floor((keyword_length - DataConfig.kFftLength + DataConfig.kHopLength) /
                 DataConfig.kHopLength))

    num_workers = cpu_count() if cpu_usage is None else int(cpu_usage)

    train_output_path = join(output_path, 'train')
    valid_output_path = join(output_path, 'valid')

    # Augment the training dataset
    augment_data(
        dataset_path['train'],
        train_output_path,
        keyword_length,
        augment_noise=augment_noise,
        noise_dir=noise_profiles,
        noise_gain=noise_gain,
        num_workers=num_workers,
        tqdm=tqdm,
        valid_mode=False)
    # Augment the validation dataset
    augment_data(
        dataset_path['valid'],
        valid_output_path,
        keyword_length,
        augment_noise=augment_noise,
        noise_dir=noise_profiles,
        noise_gain=noise_gain,
        num_workers=num_workers,
        tqdm=tqdm,
        valid_mode=True)

    features_path = join(output_path, 'features')
    remove_create_dir(features_path)
    train_feature_path = join(features_path, 'train.hdf5')
    valid_feature_path = join(features_path, 'valid.hdf5')

    # Extract the features for training dataset
    extract_features(train_output_path, train_feature_path, keyword_id_map)
    # Extract the features for validation dataset
    extract_features(valid_output_path, valid_feature_path, keyword_id_map)


def preprocess_data_denoise(dataset_path,
                    output_path,
                    noise_profiles,
                    keyword_duration,
                    cpu_usage=None):
    """Augment the datasets and extract features from it."""
    keyword_length = int(keyword_duration * DataConfig.kSampleRate / 1000)

    keyword_length = int(keyword_duration * DataConfig.kSampleRate / 1000)
    DataConfig.kWindowLength = int(
        np.floor((keyword_length - DataConfig.kFftLength + DataConfig.kHopLength) /
                 DataConfig.kHopLength))

    num_workers = cpu_count() if cpu_usage is None else int(cpu_usage)

    train_output_path = join(output_path, 'train')
    valid_output_path = join(output_path, 'valid')

    # Augment the training dataset
    augment_data_denoise(
        dataset_path['train'],
        train_output_path,
        keyword_length,
        noise_dir=noise_profiles,
        num_workers=num_workers,
        tqdm=tqdm,
        valid_mode=False)


def augment_data_denoise(in_dir,
                 out_dir,
                 keyword_length,
                 noise_dir=None,
                 num_workers=1,
                 tqdm=lambda x: x,
                 valid_mode=False):
    """Augment the audio files with time shift, time stretch, pitch shift, dynamic range
    compression (DRC) to create an augmented dataset.
    """
    logging.debug('==================================================================')
    logging.debug('||                     A U G M E N T I N G                      ||')
    logging.debug('==================================================================')
    logging.debug('Saving augmented dataset to {}'.format(out_dir))

    sample_rate = DataConfig.kSampleRate

    kTimeShiftSteps = [0, 400]    #TODO: Decide with shift length and audio window length
    kTimeStretchRate = [0.95, 1.05, 1.10, 1.07, 1.15]    #TODO: Decide with time_stretch_rate
    kPitchShiftSteps = [-2, -1, 1, 2]
    kPitchShiftSteps2 = [-3.5, -2.5, 2.5, 3.5]
    kDrcPresets = [
    # radio
        ['0.01,1', '-90,-90,-70,-70,-60,-20,0,0', '-5'],
    # film standard
        ['0.1,0.3', '-90,-90,-70,-64,-43,-37,-31,-31,-21,-21,0,-20', '0', '0', '0.1'],
    # music standard
        ['0.1,0.3', '-90,-90,-70,-58,-55,-43,-31,-31,-21,-21,0,-20', '0', '0', '0.1'],
    # speech
        ['0.1,0.3', '-90,-90,-70,-55,-50,-35,-31,-31,-21,-21,0,-20', '0', '0', '0.1']
    ]
    # bg_noises = []
    # if augment_noise:
    #     if noise_dir is not None:
    #         for bgn_filename in glob(join(noise_dir, '*.wav')):
    #             bg_y = read_audio_file(bgn_filename, sample_rate)
    #             bg_noises.append(bg_y)
    #     if noise_gain is None:
    #         noise_gain = {'min': 0.1, 'max': 0.4}

    remove_create_dir(out_dir)
    wav_out_dir = join(out_dir, 'wav')
    remove_create_dir(wav_out_dir)
    remove_create_dir(join(out_dir, 'jams'))

    executor = ProcessPoolExecutor(max_workers=num_workers)
    futures = []
    index = 0

    # # Save noise samples as unknownkeywords class
    # for bg_noise in tqdm(bg_noises, desc='Noises    ', dynamic_ncols=True):
    #     index += 1
    #     label = 'unknownkeywords-noise'
    #     for i, start in enumerate(range(0, len(bg_noise) - keyword_length, keyword_length // 2)):
    #         futures.append(
    #             executor.submit(
    #                 partial(_save_chunk, wav_out_dir, index, i,
    #                         bg_noise[start:start + keyword_length], sample_rate, label)))

    # Skip augmenting unknownkeywords in validation mode.
    skip_class = ['unknownkeywords'] if valid_mode else []

    for wavfile in tqdm(
            sorted(glob(join(in_dir, '*/*.wav'))), desc='Loading   ', dynamic_ncols=True):
        index += 1
        class_name = wavfile.split('/')[-2]
        label = class_name + '-' + wavfile.split('/')[-1].split()[0].split('.')[0]

        # Load the waveform 'y' with sample rate
        y = read_audio_file(wavfile, sample_rate, duration=None)

        print(label)

    #     # Save original waveform
    #     futures.append(
    #         executor.submit(partial(_save_orig, wav_out_dir, y, sample_rate, index, label)))

    #     if class_name in skip_class:
    #         continue

    #     ts_index = 1
    #     ps_index = 1
    #     bgn_index = 1

    #     # Time Shift waveform
    #     for i, shift in enumerate(kTimeShiftSteps):
    #         ys = y[shift:]
    #         if shift != 0:
    #             futures.append(
    #                 executor.submit(
    #                     partial(_time_shift, wav_out_dir, index, i, y[shift:], sample_rate, label)))

    #         # Time Stretch the waveform
    #         for j, rate in enumerate(kTimeStretchRate):
    #             futures.append(
    #                 executor.submit(
    #                     partial(_time_stretch, wav_out_dir, index, ts_index, ys, sample_rate, rate,
    #                             label)))
    #             ts_index += 1

    #         # Pitch Shift the waveform
    #         for steps in (kPitchShiftSteps + kPitchShiftSteps2):
    #             futures.append(
    #                 executor.submit(
    #                     partial(_pitch_shift, wav_out_dir, index, ps_index, ys, sample_rate, steps,
    #                             label)))
    #             ps_index += 1

    #         if augment_noise:
    #             # Background Noise augmentation - Augment random noise
    #             if DataConfig.kNumNoiseAugmentations:
    #                 bgn_choices = np.random.choice(
    #                     len(bg_noises), DataConfig.kNumNoiseAugmentations, replace=False)
    #             else:
    #                 bgn_choices = range(len(bg_noises))
    #             for j in bgn_choices:
    #                 futures.append(
    #                     executor.submit(
    #                         partial(_bg_noise, wav_out_dir, index, bgn_index, ys, sample_rate,
    #                                 bg_noises[j], noise_gain, label)))
    #                 bgn_index += 1

    #     # Dynamic Range Compression (DRC)
    #     for i, preset in enumerate(kDrcPresets):
    #         futures.append(
    #             executor.submit(partial(_drc, wav_out_dir, index, i, wavfile, preset, label)))

    # # Collect results of each augmentation and save the metadata
    # metadata = [future.result() for future in tqdm(futures, desc='Augmenting', dynamic_ncols=True)]
    # write_metadata(metadata, out_dir, sample_rate)
    # logging.debug('Saved augmented dataset to {}'.format(out_dir))
    # logging.debug('Saved metadata of augmented dataset in {}'.format(out_dir))