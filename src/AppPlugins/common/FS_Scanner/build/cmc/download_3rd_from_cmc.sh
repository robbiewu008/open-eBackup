#!/bin/bash
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
source ${SCRIPT_PATH}/../common/common_artget.sh
SCRIPT_NAME="${BASH_SOURCE[0]##*/}"
OPEN_SRC_INNER_PACKET_PATH=${NAS_ROOT_DIR}/third_open_src/output_pkg

PRODUCT=$1
CODE_BRANCH=$2
if [ -z "${PRODUCT}" ]; then
    PRODUCT="dorado"
fi

if [ -z "${SCANNER_BRANCH}" ]; then
    SCANNER_BRANCH="BR_Dev"
fi
function download_scanner_3rd()
{
    log_echo "begin download 3rd from cmc"
    download_scanner_3rd_4_cmc ${PRODUCT} ${SCANNER_BRANCH}
    if [ $? -ne 0 ]; then
        log_echo "download artifact error"
        exit 1
    fi
}

function main()
{
    download_scanner_3rd "%@"
    return $?
}

main "$@"
exit $?