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
copy_path="$1/AppPlugins_NAS"

function main()
{
    echo $copy_path
    cd ${FILE_ROOT_DIR}/../..

    mkdir -p $copy_path
    mkdir -p $copy_path/plugins

    cp -r $REPO_ROOT_PATH/plugins/file $copy_path/plugins
    cp -r $REPO_ROOT_PATH/framework $copy_path
    cp -r $REPO_ROOT_PATH/Module $copy_path
    cp -r $REPO_ROOT_PATH/FS_Backup $copy_path
}

main "$@"
exit $?