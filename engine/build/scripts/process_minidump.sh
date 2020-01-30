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

WORK_DIR=/tmp/minidump
SYMBOL_DIR=$WORK_DIR/syms

mkdir -p $SYMBOL_DIR
if [ $# -lt 1 ]; then
    printf "Usage: process_minidump <minidump_file>\n"
    exit
fi

MINIDUMP_FILE="$1"

# Extracts stacktrace
$WORK_DIR/minidump_stackwalk $MINIDUMP_FILE $SYMBOL_DIR/ &2>/dev/null