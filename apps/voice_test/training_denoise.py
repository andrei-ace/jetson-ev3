'''
Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
'''

import os
os.environ['CUDA_DEVICE_ORDER'] = 'PCI_BUS_ID'    # so the IDs match nvidia-smi
os.environ['CUDA_VISIBLE_DEVICES'] = '0'    # Limit the GPU usage to gpu #0

import argparse
import json
import h5py
import logging
import numpy as np
import time
from os.path import join

import tensorflow as tf
from tensorflow.python.framework import graph_util, graph_io
from tensorflow.python.tools import freeze_graph

from keras import backend as K
from keras.backend.tensorflow_backend import set_session
from keras.models import Model, load_model


from utils import create_dir, DataConfig, preprocess_data_denoise, remove_create_dir

# Dict of arguments that will be parsed from config and command line
args = None
# Default values of arguments
kArgsDefaults = {
    'config_filename': 'training.config.json',
    'tmpdir': '/tmp',
    'model_output_path': 'model',
    'training_epochs': 100,
    'batch_size': 32,
    'restrictive': False,
    'learning_rate': 1e-5,
    'epoch_number': 0
}
# Name of the checkpoint file
kCheckpointFileName = 'isaac_vcd_model'
# Name of the generated metadata file
kMetadataFileName = 'isaac_vcd_model.metadata.json'
# Name format of the intermediate keras models
kCheckpointAllFileName = 'isaac_vcd_model_{epoch:02d}_{loss:4f}_{val_loss:.4f}_{val_accuracy:.4f}.h5'
# Op names
kOutputOpName = 'output_node'

def check_args(key, value1, value2=None, value3=None):
    """Check if a value is available and print an error if it is missing"""
    value = value1 or value2 or value3
    if value is None:
        raise ValueError('Missing configuration for {}'.format(key))
    return value

def parse_config():
    """Parse the config.json file and compare with the command line arguments to populate the args.
    Command line arguments are prioritised over the config file.
    """
    global args
    config = json.loads('{}')
    config_filename = args.config_filename or os.path.exists(kArgsDefaults['config_filename'])
    if config_filename:
        with open(config_filename) as f:
            config = json.load(f)

    args.train_dataset_path = check_args('train_dataset_path', args.train_dataset_path,
                                         config.get('train_dataset_path'))
    args.validation_dataset_path = check_args('validation_dataset_path',
                                              args.validation_dataset_path,
                                              config.get('validation_dataset_path'))
    args.noise_profile_path = args.noise_profile_path or config.get('noise_profile_path')
    args.tmpdir = check_args('tmpdir', args.tmpdir, config.get('tmpdir'), kArgsDefaults['tmpdir'])
    args.tmpdir = join(args.tmpdir, 'isaac_voice_command_training')

    args.logdir = check_args('logdir', args.logdir, config.get('logdir'), join(args.tmpdir, 'logs'))
    args.model_output_path = check_args('model_output_path', args.model_output_path,
                                        config.get('model_output_path'),
                                        kArgsDefaults['model_output_path'])

    args.training_epochs = check_args('training_epochs', args.training_epochs,
                                      config.get('training_epochs'),
                                      kArgsDefaults['training_epochs'])
    args.batch_size = check_args('batch_size', args.batch_size, config.get('batch_size'),
                                 kArgsDefaults['batch_size'])
    args.restrictive = check_args('restrictive', args.restrictive, config.get('restrictive'),
                                  kArgsDefaults['restrictive'])
    args.learning_rate = check_args('learning_rate', args.learning_rate,
                                    config.get('learning_rate'), kArgsDefaults['learning_rate'])

    args.checkpoint = args.checkpoint or config.get('checkpoint')
    args.epoch_number = check_args('epoch_number', args.epoch_number, config.get('epoch_number'),
                                   kArgsDefaults['epoch_number'])
    args.gpu_memory_usage = args.gpu_memory_usage or config.get('gpu_memory_usage')


def parse_command_line_arguments():
    """Define and parse the command line arguments of the binary into args"""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-t',
        '--train_dataset_path',
        dest='train_dataset_path',
        help='Where the training dataset is stored.')
    parser.add_argument(
        '--validation_dataset_path',
        dest='validation_dataset_path',
        help='Where the validation dataset is stored.')
    parser.add_argument(
        '--noise_profile_path',
        dest='noise_profile_path',
        help='Where the noise profiles are stored.')
    parser.add_argument(
        '--tmpdir',    # default='/tmp',
        dest='tmpdir',
        help='Where the processed data and checkpoints are temporarily stored. Default: /tmp')
    parser.add_argument(
        '--logdir',
        dest='logdir',
        help='Where training logs are stored for Tensorboard usage. Default is <tmpdir>/logs')
    parser.add_argument(
        '-o',
        '--model_output_path',    # default='model',
        dest='model_output_path',
        help='Where the trained model and metadata are stored. Default: <current_dir>/model')
    parser.add_argument(
        '--keyword_duration',
        type=float,    # default='500ms',
        dest='keyword_duration',
        help='Duration of keywords in seconds. Range is [0.1, 1]. Default: 0.5')
    parser.add_argument(
        '--training_epochs',
        type=int,    # default=100,
        dest='training_epochs',
        help='Number of epochs to run the training. Default: 100')
    parser.add_argument(
        '--batch_size',
        type=int,    # default=32,
        dest='batch_size',
        help='Batch size used for training. Default: 32')
    parser.add_argument(
        '--restrictive',
        action='store_true',
        dest='restrictive',
        help='Train the model limited number of speaker. Disabled by default.')
    parser.add_argument(
        '--learning_rate',
        '--lr',
        type=float,    # default=1e-5,
        dest='learning_rate',
        help='Learning rate used for Adamax optimizer. Default: 1e-5')
    parser.add_argument(
        '--checkpoint',
        dest='checkpoint',
        help='Keras checkpoint to be loaded to continue training. ' +
        'Defaults to not loading checkpoints.')
    parser.add_argument(
        '-e',
        '--epoch_number',
        type=int,    # default=0,
        dest='epoch_number',
        help='Epoch at which to start training when resuming from checkpoint. Default: 0')
    parser.add_argument(
        '--gpu_memory_usage',
        type=float,
        dest='gpu_memory_usage',
        help='Specified to limit the usage of gpu memory. Default: 0 (no limit)')

    parser.add_argument(
        '--config_filename',    # default='training.config.json',
        dest='config_filename',
        help='Path to json file with configuration parameters. ' +
        'However command line arguments are prioritised. Default: training.config.json if exists.')

    global args
    args = parser.parse_args()



def main():
    logging.basicConfig(
        format='%(asctime)s %(levelname)s: %(message)s',
        datefmt='%m/%d/%Y %I:%M:%S %p',
        level=logging.DEBUG)
    # Parse the command line arguments into args
    parse_command_line_arguments()

    # Parse the config.json file
    parse_config()
    logging.debug('Parse arguments: {}'.format(args))

    # Limit the GPU memory usage.
    tf_config = tf.ConfigProto()
    if args.gpu_memory_usage:
        tf_config.gpu_options.per_process_gpu_memory_fraction = args.gpu_memory_usage
    else:
        tf_config.gpu_options.allow_growth = True
    sess = tf.Session(config=tf_config)
    set_session(sess)

    if args.checkpoint is None:
        None
        # Augment the dataset and extract the features in preprocessing stage
        preprocess_data_denoise({'train': args.train_dataset_path,
                         'valid': args.validation_dataset_path},
                        args.tmpdir,
                        args.noise_profile_path,
                        args.keyword_duration,
                        cpu_usage=1)


if __name__ == '__main__':
    main()
