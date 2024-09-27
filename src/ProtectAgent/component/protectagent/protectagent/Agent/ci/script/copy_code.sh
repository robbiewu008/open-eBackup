#!/bin/bash
CURRENT_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
set -ex
DesPath=$1
SrcPath=${CURRENT_PATH}/../../../../protectagent
 
cp -rf "${SrcPath}" "${DesPath}/ProtectAgent/component/protectagent"
echo "Copy code success."