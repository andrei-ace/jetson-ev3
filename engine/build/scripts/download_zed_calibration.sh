#!/bin/sh
#####################################################################################
# Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto. Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.
#####################################################################################
# Helper script to download the factory calibration for the zed camera
# Usage : download_zed_calibration.sh -s <zed_camera_serial_number>

ZED_SERIAL=$2
BASE_URL="https://www.stereolabs.com/developers/calib/"
SETTINGS_FILE=SN${ZED_SERIAL}.conf

if [ "$1" = "-s" ] && [ ! -z "$ZED_SERIAL" ]; then
    curl -s ${BASE_URL}?SN=${ZED_SERIAL} -o ${SETTINGS_FILE}
    echo "${SETTINGS_FILE} downloaded.\n"
    head -10 ${SETTINGS_FILE}
    echo "..."
else
    echo "Invalid arguments"
    echo "Usage : download_zed_calibration.sh -s <zed_camera_serial_number>"
    echo "Example : download_zed_calibration.sh -s 12345"
fi