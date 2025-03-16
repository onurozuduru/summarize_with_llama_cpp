#!/bin/bash
###############################################################################
#File: start_with_devcontainer_cli.sh
#
#License: MIT
#
#Copyright (C) 2025 Onur Ozuduru
#
#Follow Me!
#  github: github.com/onurozuduru
###############################################################################

SCRIPT_DIR=$(dirname "$(realpath "$0")")
WORKSPACE_DIR=$(dirname "$SCRIPT_DIR")
REBUILD=""

print_help() {
	echo "Usage: $0 [--rebuild] [-h | --help]"
	echo -e "Create or start the container by using devcontainer cli."
	echo -e "\t--rebuild\tRemove existing container and rebuild."
	echo -e "\t-h,--help\tDisplay help."
}

set -e
params=$(getopt -l "help,rebuild" -o "h" -- "$@")

eval set -- "$params"

while true; do
	case $1 in
	-h | --help)
		print_help
		exit 0
		;;
	--rebuild)
		REBUILD="--remove-existing-container"
		;;
	--)
		shift
		break
		;;
	*)
		print_help
		exit 0
		;;
	esac
	shift
done

if ! [ -x "$(command -v devcontainer)" ]; then
	echo "Devcontainer cli is not installed!"
	exit 1
fi

MOUNT_X11=(--mount "type=bind,source=/tmp/.X11-unix,target=/tmp/.X11-unix")
CONTAINER_RESULT=$(devcontainer up --workspace-folder "$WORKSPACE_DIR" "${MOUNT_X11[@]}" $REBUILD)
CONTAINER_ID=$(echo "$CONTAINER_RESULT" | sed 's/\(.*containerId":"\)\(.*\)\(","remoteUser.*\)/\2/')

if [ -z "$CONTAINER_ID" ]; then
	echo "Container error: ${CONTAINER_ID}"
else
	if [[ -z "$TZ" ]]; then
		TZ=$(cat /etc/timezone)
	fi
	devcontainer exec --container-id "$CONTAINER_ID" --remote-env "TZ=$TZ" --remote-env "DISPLAY=$DISPLAY" bash -c "[ -f ~/additional_scripts/tmux_env.sh ] && ~/additional_scripts/tmux_env.sh -N -w ~/workspace || bash"
	docker kill "$CONTAINER_ID"
fi
