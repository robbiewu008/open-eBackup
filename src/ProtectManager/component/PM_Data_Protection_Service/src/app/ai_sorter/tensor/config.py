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
from typing import List

import torch
from pydantic import BaseSettings



class Options(BaseSettings):
    n_epochs: int = 1000
    n_patients: int = 5
    lr_model: float = 1e-4

    schedule_policy: int = 0
    mlq_priorities: List[int] = [0, 1, 4]

    batch_sizes: List[int] = [8, 16]
    hidden_sizes: List[int] = [64, 128, 256]
    lstm_layers: List[int] = [1, 2, 3]

    loss_function = torch.nn.MSELoss()
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    save_dir: str = os.path.abspath(os.path.join(os.getcwd(), "output"))
    tmp_model_name: str = "bestModelTrain.pt"
    tmp_model_path: str = os.path.join(save_dir, tmp_model_name)
    final_model_name: str = "bestModel.pt"
    final_model_path: str = os.path.join(save_dir, final_model_name)


opt = Options()
