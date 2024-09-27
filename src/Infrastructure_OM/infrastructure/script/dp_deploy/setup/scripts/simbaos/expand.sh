#!/bin/bash
set -e

cd $(dirname ${BASH_SOURCE[0]})
source ../common.sh

sudo smartkube expand \
    --ignorephase="node precheck" \
    --folder=${SIMBAOS_PACKAGE_PATH}