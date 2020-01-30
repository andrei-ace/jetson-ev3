#!/bin/bash
#####################################################################################
# Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto. Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.
#####################################################################################

set +e

UNAME=nvidia
# get command line arguments
while [ $# -gt 0 ]; do
  case "$1" in
    -h|--host)
      HOST="$2"
      ;;
    -u|--user)
      UNAME="$2"
      ;;
    *)
      printf "Error: Invalid arguments: %1 %2\n"
      exit 1
  esac
  shift
  shift
done

if [[ -z $HOST ]]; then
  echo "Error: Jetson device IP must be specified with -h IP."
  exit 1
fi

# This function will be ran on the target device
remote_function() {
  # Install packages
  sudo apt update && sudo apt install -y rsync curl libhidapi-libusb0 libturbojpeg python3-pip

  # pycapnp is required to run Python Codelets.
  echo "Installing pycapnp. This may take about five minutes to complete. Please wait..."
  python3 -m pip install --user pycapnp

  # psutil is needed for navigation evaluation
  python3 -m pip install psutil

  # Give user permission to use i2c and tty devices
  sudo usermod -a -G i2c,dialout $USER

  # Blacklist nvs_bmi160 kernel mod
  echo blacklist nvs_bmi160 | sudo tee /etc/modprobe.d/blacklist-nvs_bmi160.conf > /dev/null

  sudo shutdown -r now
}

# Installs dependencies on Jetson devices
ssh -t $UNAME@$HOST "$(declare -pf remote_function); remote_function"

echo "Rebooting the Jetson device"
