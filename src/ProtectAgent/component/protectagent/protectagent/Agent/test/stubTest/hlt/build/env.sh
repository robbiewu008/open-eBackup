export AGENT_ROOT=${HOME}/Agent
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
export HLT_ROOT=${AGENT_ROOT}/test/stubTest/hlt
export PATH=.:${PATH}
if [ -z ${LD_LIBRARY_PATH} ]
then
    export LD_LIBRARY_PATH=${AGENT_ROOT}/bin
else
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${AGENT_ROOT}/bin
fi

PS1="\h [\u]:\w # "
alias h="history"
alias ll="ls -l"
alias lsa="ls -al"
alias dir="ls -lF"

alias mk="sh ${AGENT_ROOT}/test/stubTest/hlt/build/hlt_make.sh $*"

chmod 0755 ${AGENT_ROOT}/test/stubTest/hlt/build/*.sh

