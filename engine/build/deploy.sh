#!/bin/bash
#####################################################################################
# Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto. Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.
#####################################################################################

# constants
BAZEL_BIN="bazel-bin"
REQUESTED_UBUNTU_VERSION="18.04"
REQUESTED_BAZEL_VERSION="0.19.2"
LOCAL_DEVICE="x86_64"
COLOR_RED='\033[0;31m'
COLOR_GREEN='\033[0;32m'
COLOR_NONE='\033[0m'
COLOR_YELLOW='\033[0;33m'

# Helper functions to be used in this script.
# Prints the error message and exits the script.
error_and_exit() {
  printf "${COLOR_RED}Error: $1${COLOR_NONE}\n"
  exit 1
}
# Prints the warning message.
warn() {
  printf "${COLOR_YELLOW}Warning: $1${COLOR_NONE}\n"
}

# Check Ubuntu and bazel version before building.
# Theses checks should reduce the number of problems reported on forums.
UBUNTU_VERSION=$(lsb_release -rs)
BAZEL_VERSION=$(bazel version | grep 'Build label' | sed 's/Build label: //')
if [[ $UBUNTU_VERSION != $REQUESTED_UBUNTU_VERSION ]]; then
  error_and_exit \
    "Isaac currently only supports Ubuntu $REQUESTED_UBUNTU_VERSION. Your Ubuntu version is $UBUNTU_VERSION."
fi
if [[ $BAZEL_VERSION != $REQUESTED_BAZEL_VERSION ]]; then
  error_and_exit \
    "Isaac requires bazel version $REQUESTED_BAZEL_VERSION. Please verify your bazel with 'bazel version' command."
fi

# used arguments with default values
UNAME=$USER
REMOTE_USER=nvidia
REMOTE_USER_SET=false

# get command line arguments
while [ $# -gt 0 ]; do
  case "$1" in
    -p|--package)
      PACKAGE="$2"
      ;;
    -d|--device)
      DEVICE="$2"
      ;;
    -h|--host)
      HOST="$2"
      ;;
    -u|--user)
      UNAME="$2"
      ;;
    -s|--symbols)
      NEED_SYMBOLS="True"
      shift
      continue
      ;;
    -r|--run)
      NEED_RUN="True"
      shift
      continue
      ;;
    --remote_user)
      REMOTE_USER="$2"
      REMOTE_USER_SET=true
      ;;
    --deploy_path)
      DEPLOY_PATH="$2"
      ;;
    *)
      printf "Error: Invalid arguments: ${1} ${2}\n"
      exit 1
  esac
  shift
  shift
done

if [ -z "$PACKAGE" ]; then
  error_and_exit "Package must be specified with -p //foo/bar:tar."
fi
if [[ $PACKAGE != //* ]]; then
  error_and_exit "Package must start with //. For example: //foo/bar:tar."
fi

if [ -z "$HOST" ]; then
  error_and_exit "Host IP must be specified with -h IP."
fi

if [ -z "$DEVICE" ]; then
  error_and_exit "Desired target device must be specified with -d DEVICE. Valid choices: 'jetpack42', 'x86_64'."
fi

#Check if we need ssh to deploy. Potentially overwrite REMOTE_USER.
SSH_NEEDED=true
if [[ $HOST == "localhost" || $HOST == "127.0.0.1" ]]; then
  # Check username
  if [[ $REMOTE_USER_SET == false ]]; then
    # If user has not explicitly set REMOTE_USER, set it to $USER
    echo "No remote user is specified. Using '$USER' for local deployment."
    REMOTE_USER=$USER
  elif [[ $REMOTE_USER != $USER ]]; then
    warn "This is a local deployment, but remote user is explicitly specified as '$REMOTE_USER'"
  fi
  if [[ $REMOTE_USER == $USER ]]; then
    SSH_NEEDED=false
  fi
  if [[ $DEVICE != $LOCAL_DEVICE ]]; then
    warn "Deploying a '$DEVICE' package to localhost"
  fi
fi

# Split the target of the form //foo/bar:tar into "//foo/bar" and "tar"
targetSplitted=(${PACKAGE//:/ })
if [[ ${#targetSplitted[@]} != 2 ]]; then
  error_and_exit "Package '$PACKAGE' must have the form //foo/bar:tar"
fi
PREFIX=${targetSplitted[0]:2}
TARGET=${targetSplitted[1]}

# check if multiple potential output files are present and if so delete them first
TAR1="$BAZEL_BIN/$PREFIX/$TARGET.tar"
TAR2="$BAZEL_BIN/$PREFIX/$TARGET.tar.gz"
if [[ -f $TAR1 ]] && [[ -f $TAR2 ]]; then
  rm -f $TAR1
  rm -f $TAR2
fi

echo "================================================================================"
echo "Building Minidump tools"
echo "================================================================================"
source engine/build/scripts/prepare_minidump_tools.sh && wait

# build the bazel package
echo "================================================================================"
echo "Building //$PREFIX:$TARGET for target platform '$DEVICE'"
echo "================================================================================"
bazel build --config $DEVICE $PREFIX:$TARGET --strip=always || exit 1

# Find the filename of the tar archive. We don't know the filename extension so we look for the most
# recent file and take the corresponding extension. We accept .tar or .tar.gz extensions.
if [[ -f $TAR1 ]] && [[ $TAR1 -nt $TAR2 ]]; then
  EX="tar"
elif [[ -f $TAR2 ]] && [[ $TAR2 -nt $TAR1 ]]; then
  EX="tar.gz"
else
  error_and_exit "Package '$PACKAGE' did not produce a .tar or .tar.gz file"
fi
TAR="$TARGET.$EX"

# Print a message with the information we gathered so far
echo "================================================================================"
echo "Deploying //$PREFIX:$TARGET ($EX) to $REMOTE_USER@$HOST under name '$UNAME'"
echo "================================================================================"

# unpack the package in the local tmp folder
rm -f /tmp/$TAR
cp $BAZEL_BIN/$PREFIX/$TAR /tmp/
rm -rf /tmp/$TARGET
mkdir /tmp/$TARGET
tar -xf /tmp/$TAR -C /tmp/$TARGET
# fixing libary loading issues where libraries from _solib_arm64-v8a are loaded before the Isaac libraries
mv /tmp/$TARGET/_solib_arm64-v8a/ /tmp/$TARGET/external/com_nvidia_isaac/

# Deploy directory
if [ -z "$DEPLOY_PATH" ]
then
  DEPLOY_PATH="/home/$REMOTE_USER/deploy/$UNAME/"
fi

# sync the package folder to the remote
REMOTE_USER_AND_HOST="$REMOTE_USER@$HOST:"
# Special case: Don't ssh if not needed
if [[ $SSH_NEEDED == false ]]; then
  REMOTE_USER_AND_HOST=""
fi
rsync -avz --delete --checksum --rsync-path="mkdir -p $DEPLOY_PATH/ && rsync" \
    /tmp/$TARGET $REMOTE_USER_AND_HOST$DEPLOY_PATH
status=$?
if [ $status != 0 ]; then
  error_and_exit "rsync failed with exit code $status"
fi

if [[ -z $NEED_SYMBOLS ]]; then
  echo "================================================================================"
  echo "To grab symbols pass -s/--symbols"
  echo "================================================================================"
else
  echo "================================================================================"
  echo "Grabbing symbols"
  echo "================================================================================"
  # Retain symbols in all binaries
  bazel build --config $DEVICE $PREFIX:$TARGET --strip=never || exit 1

  # Unpack the package in the local tmp folder
  rm -f /tmp/$TAR
  cp $BAZEL_BIN/$PREFIX/$TAR /tmp/
  rm -rf /tmp/$TARGET
  mkdir /tmp/$TARGET
  tar -xf /tmp/$TAR -C /tmp/$TARGET
  # fixing libary loading issues where libraries from _solib_arm64-v8a are loaded before the Isaac libraries
  mv /tmp/$TARGET/_solib_arm64-v8a/ /tmp/$TARGET/external/com_nvidia_isaac/

  EXECUTABLES=$(find /tmp/$TARGET -executable -type f)
  for exe in $EXECUTABLES
  do
    if [[ ! -z $(file "$exe" | sed '/ELF/!d') ]]; then
      engine/build/scripts/process_syms.sh $exe
    fi
  done
  wait
fi
echo "================================================================================"
printf "${COLOR_GREEN}Deployed${COLOR_NONE}\n"
echo "================================================================================"

if [[ ! -z $NEED_RUN ]]; then
  echo "================================================================================"
  echo "Running on Remote"
  echo "================================================================================"
  # echo "cd $DEPLOY_PATH/$TARGET; ./$PREFIX/${TARGET::-4}"
  ssh -t $REMOTE_USER@$HOST "cd $DEPLOY_PATH/$TARGET; ./$PREFIX/${TARGET::-4}"
fi
