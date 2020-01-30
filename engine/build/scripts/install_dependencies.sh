#!/bin/bash
#####################################################################################
# Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto. Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.
#####################################################################################

# Only for debug purposes
# set -ex

if ! [ -x "$(command -v sudo)" ]; then
  sudo(){
    PYTHON="python3 -m pip install"
    eval "$@"
  }
else
  PYTHON="python3 -m pip install --user"
fi

install_package(){
 echo -e "\e[32m[INFO] Installing packages - $@\e[0m"
 sudo apt-get install -y $@ -qq > /dev/null
 if [ $? -ne 0 ]; then
   echo -e "\e[31m[ERROR] Failed to install packages - $@\e[0m"
   exit 1
 fi
 echo -e "\e[32m[INFO] Successfully installed packages - $@\e[0m"
}

# Updates the list of available packages and their versions to get up-to-date packages
sudo apt-get update
# Installs C++ dev tools
install_package git git-lfs git-review build-essential g++
git lfs install
# Installs Bazel deps
install_package pkg-config zip zlib1g-dev unzip curl
# Installs core deps
install_package uuid-dev
# Installs Python (for python bridge)
install_package python python-dev
# Installs 3D app deps
install_package libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev
# Installs audio DL training app deps
install_package sox
# Installs ARM building deps
install_package gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
# Installs codecs
install_package libvpx5 libtwolame-dev libtheora-bin libspeex1
install_package libopus0 libmp3lame0 libvdpau1 openexr libmpg123-0
# Install Sphinx deps
install_package python3-pip \
    latexmk texlive-latex-recommended texlive-latex-extra texlive-fonts-recommended \
    texlive-luatex texlive-xetex
# Install Zed Camera deps
install_package libudev-dev
# Install I2C deps
install_package libi2c-dev
# Install Realsense Camera dependencies
install_package libhidapi-libusb0 libturbojpeg
# Installs python3
install_package python3-dev libssl-dev
install_package python-tk libopenmpi-dev
# Install Sphinx (documentation generation)
install_package python3-sphinx python3-sphinx-rtd-theme
# Install GLib and GStreamer for open source GStreamer and NVIDIA DeepStream components.
install_package glib2.0 gstreamer1.0-plugins.* libgstreamer1.0.* libgstreamer-plugins-.*1.0.*

# Install Sphinx (documentation generation)
$PYTHON --upgrade pip
$PYTHON launchpadlib
$PYTHON --upgrade setuptools
$PYTHON -r engine/build/scripts/requirements.txt

# Baselines needs to be installed after requirements.txt as it is not presenting dependency on tensorflow properly
# MIT License : https://github.com/openai/baselines/blob/master/LICENSE
$PYTHON git+git://github.com/openai/baselines.git@a071fa7630288ee00c86e140c5e674061f64690d#egg=baselines

# Install/setup pyparsing for Python3.
$PYTHON pyparsing

# Installs Bazel
TMPFOLDER=$(mktemp -d)
OUTPUT_SH=$TMPFOLDER/bazel.sh
curl -s -L https://github.com/bazelbuild/bazel/releases/download/0.19.2/bazel-0.19.2-installer-linux-x86_64.sh -o $OUTPUT_SH || exit 1
chmod +x $OUTPUT_SH || exit 1
sudo bash $OUTPUT_SH || exit 1

echo "Installation Succeeded"
