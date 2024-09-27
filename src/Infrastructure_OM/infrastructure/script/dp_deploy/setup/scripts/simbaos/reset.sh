#!/bin/bash
set -e
cd $(dirname ${BASH_SOURCE[0]})
source ../common.sh

if command -v smartkube &> /dev/null; then
    sudo smartkube reset \
        --local \
        --phase=all \
        --folder=${SIMBAOS_PACKAGE_PATH}
fi