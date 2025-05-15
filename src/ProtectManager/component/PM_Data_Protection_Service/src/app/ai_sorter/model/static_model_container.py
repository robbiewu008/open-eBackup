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

from app.ai_sorter.model.torch_module import LSTM


class StaticAIModelContainer(object):
    """
    这里的5个变量都为静态变量
    :param ai_model: AI模型吗，如果ai_model为None，则表示未加载模型
    :param norm_paras: 归一化参数
    :param retrain_time: 重训时间
    :param training_lock: 锁，用于训练模型时使用
    :param model_load_lock: 锁，用于加载模型时使用
    """
    ai_model: LSTM = None
    norm_paras = [0, 0]
    retrain_time = 0
    training_lock = RLock()
    model_load_lock = RLock()
