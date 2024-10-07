BRANCH_COV_THRESHOLD=49
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
FUNCTIONS_COV_THRESHOLD=93
LINES_COV_THRESHOLD=92

function check_coverage()
{
    local branches_cov=$(cat coverage.log | grep branches | awk -F' ' '{print $2}' | awk -F'%' '{print $1}')
    local functions_cov=$(cat coverage.log | grep functions | awk -F' ' '{print $2}' | awk -F'%' '{print $1}')
    local lines_cov=$(cat coverage.log | grep lines | awk -F' ' '{print $2}' | awk -F'%' '{print $1}')
    echo "branches_cov = $branches_cov%, functions_cov = $functions_cov%, lines_cov = $lines_cov%"

    if [ $(echo $branches_cov"<"$BRANCH_COV_THRESHOLD | bc -l) -eq 1 ]
    then
        echo "branch coverage $branches_cov% unqualified, need $BRANCH_COV_THRESHOLD%"
        return 1
    elif [ $(echo $functions_cov"<"$FUNCTIONS_COV_THRESHOLD | bc -l) -eq 1 ]
    then
        echo "functions coverage $functions_cov% unqualified, need $FUNCTIONS_COV_THRESHOLD%"
        return 1
    elif [ $(echo $lines_cov"<"$LINES_COV_THRESHOLD | bc -l) -eq 1 ]
    then
        echo "lines coverage $lines_cov% unqualified, need $LINES_COV_THRESHOLD%"
        return 1
    fi
    echo "check_coveraged passed"
    return 0
}