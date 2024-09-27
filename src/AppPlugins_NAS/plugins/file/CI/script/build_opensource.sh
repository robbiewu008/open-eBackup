FILE_ROOT_DIR=$(cd "$(dirname ${BASH_SOURCE[0]})/../.."; pwd)
#
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
FRAMEWORK_DIR=$(cd "${FILE_ROOT_DIR}/../../framework"; pwd)
MODULE_THIRD_DIR=$(cd "${FILE_ROOT_DIR}/../../Module/third_open_src"; pwd)
FRAMEWORK_OUTPUT=${FRAMEWORK_DIR}/output_pkg
source ${FRAMEWORK_DIR}/build/common/common.sh
SCRIPT_NAME="${BASH_SOURCE[0]##*/}"
MODULE_LIB_DIR="${FILE_ROOT_DIR}/../../Module/lib"
REPO_ROOT_PATH="${FILE_ROOT_DIR}/../.."
open_source_obligation_path="${REPO_ROOT_PATH}/../../open-source-obligation/AppPlugins_NAS"
copy_path=$1

function main()
{
    system_name=$(uname -p)
    if [ "X${system_name}" != "Xaarch64" ];then
        echo "system is no aarch64"
        return 0
    fi
    cp -r $open_source_obligation_path/framework $REPO_ROOT_PATH
    cp -r $open_source_obligation_path/FS_Scanner $REPO_ROOT_PATH
    cp -r $open_source_obligation_path/Module $REPO_ROOT_PATH
    find $REPO_ROOT_PATH -name CMakeCache.txt | xargs rm -f
    ls $REPO_ROOT_PATH
    ls $REPO_ROOT_PATH/framework
    ls $REPO_ROOT_PATH/Module
    sh pack_openrepo.sh

    mkdir -p ${REPO_ROOT_PATH}/../../open-source-obligation/Plugins/Linux/aarch64
    cp $FRAMEWORK_DIR/framework/output_pkg/* ${REPO_ROOT_PATH}/../../open-source-obligation/Plugins/Linux/aarch64
}

main "$@"
exit $?