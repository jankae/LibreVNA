#!/usr/bin/env bash

set -e

PROJECT_DIR="${BASH_SOURCE%/*/*}"

SCRIPT_DIR="Software/PC_Application"

DRY_RUN=0
checkSources()
{
	START=$(date +%s)
	local DIRS=(
		.
		Calibration
		CustomWidgets
		Device
		Generator
		SpectrumAnalyzer
		Tools
		Traces
		Traces/Marker
		Traces/Math
		Traces/Math/parser
		Util
		VNA
		VNA/Deembedding
	)

	local ARGS=""
	if [ ${DRY_RUN} -eq 1 ]; then
		ARGS+="--dry-run";
		echo "Applying dry run"
	fi
	for dir in "${DIRS[@]}"
	do
		find $PROJECT_DIR/$dir -maxdepth 1 -type f -regex '.*\.\(cpp\|h\)' -exec clang-format -style=file $ARGS -i {} \;
	done
	ELAPSED=$(date +%s)
	DELTA_TIME=$((ELAPSED-START))

	echo "linting took: $DELTA_TIME seconds to complete"
}


opstr+="l-:";

while getopts "${opstr}" OPTION; do
	case $OPTION in
		l)  LINT=1; ;;
		-) case ${OPTARG} in
			dry-run) DRY_RUN=1; ;;
		   esac;;
		*) ;;
	esac
done

if [ "${LINT}" == 1 ]; then
	checkSources;
fi


exit 0
