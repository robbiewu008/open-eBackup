set -e
cd $(dirname ${BASH_SOURCE[0]})
source ../common.sh

NODE_NAME=$1

${SIMBAOS_SMARTKUBE_INSTALL_PATH}/smartkube delete_node \
    --nodeName ${NODE_NAME} \
    --certType pacific