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
    -d|--dump)
      DUMP="$2"
      ;;
    -h|--host)
      HOST="$2"
      ;;
    --remote_user)
      UNAME="$2"
      ;;
    *)
      printf "Error: Invalid arguments: %1 %2\n"
      exit 1
  esac
  shift
  shift
done

if [ -z "$DUMP" ]; then
  echo "Error: Dump file path must be specified with -d //foo/bar:tar."
  exit 1
fi
if [[ -z $HOST ]]; then
  echo "Error: Jetson device IP must be specified with -h IP."
  exit 1
fi

IFS='/' read -ra SPLITTED <<< "$DUMP"
for FILENAME in "${SPLITTED[@]}"; do
    :
done

echo "Copying minidump file '$DUMP'"
scp $UNAME@$HOST:$DUMP /tmp

engine/build/scripts/process_minidump.sh "/tmp/$FILENAME"