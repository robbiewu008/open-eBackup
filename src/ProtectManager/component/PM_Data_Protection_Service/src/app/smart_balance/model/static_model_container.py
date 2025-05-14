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
from threading import RLock
from app.smart_balance.model.torch_module import ANet


class StaticContainer(object):
    """
    :param ai_model: AI模型吗，如果ai_model为None，则表示未加载模型
    :param norm_paras1， norm_paras2: 备份数据量和备份用时的归一化参数
    :param training_lock: 锁，用于训练模型时使用
    :param model_load_lock: 锁，用于加载模型时使用
    """
    seq_model: ANet = None
    gbrt_model = None
    rf_model = None
    ada_model = None
    training_lock = RLock()
    model_load_lock = RLock()
