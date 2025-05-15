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
from pydantic import BaseSettings


class SMConst(BaseSettings):
    # the shape of features for build AI model
    flen: int = 5
    # dataset reget time
    rd_days: int = 1
    # model rebuild time
    rm_days: int = 3
    # when the predicted value bigger/smaller than the real value 5 times in 9 times
    # finetune the output prediction model
    finetune_threshold: int = 5
    # epoch save limited rounds
    epoch_rounds: int = 30
    # dataset needed page size
    dataset_pages: int = 200
    # other backuptype
    other_backuptype = 7
    # very small number, used to judge the situation when the boundary is 0
    epsilon = 1e-10