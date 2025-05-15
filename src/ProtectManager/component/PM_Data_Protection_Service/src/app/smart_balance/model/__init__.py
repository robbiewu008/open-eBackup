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
import os
import random
import torch

from app.smart_balance.model.static_model_container import StaticContainer
from app.smart_balance.model.torch_module import ANet


def seed_torch(seed=42):
    random.seed(seed)  # python seed
    os.environ['PYTHONHASHSEED'] = str(seed)  # 设置python哈希种子
    torch.manual_seed(seed)  # 为当前CPU设置随机种子
    torch.cuda.manual_seed(seed)  # 为当前GPU设置随机种子
    torch.cuda.manual_seed_all(seed)  # 使用多块GPU时，均设置随机种子
    torch.backends.cudnn.deterministic = True
