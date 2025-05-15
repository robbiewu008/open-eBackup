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
from enum import Enum
import torch
from pydantic import BaseSettings



class ModelName(Enum):
    anet = "ANet"
    gbrt = "GBRT"
    rf = "RF"
    adabr = "AdaBR"


class Options(BaseSettings):
    n_epochs: int = 10000
    n_patients: int = 10
    window_len: int = 10
    lr: float = 1e-4 * 3
    allow_corun: int = 20
    batch_sizes: int = 32
    drop_out: float = 0.05
    output_size: int = 1
    rf_criterion = "friedman_mse"
    rf_min_samples_leaf = 3
    ada_para = 100

    base_dir = "/opt/ProtectManager/smart_balance/"
    # dataset saved path
    file_dir: str = os.path.join(base_dir, "fileset")

    dataset_path: str = os.path.join(file_dir, "raw_dataset.txt")
    # record saved path
    seq_record_path: str = os.path.join(file_dir, f"predicted_record_{ModelName.anet}.txt")
    gbrt_record_path: str = os.path.join(file_dir, f"predicted_record_{ModelName.gbrt}.txt")
    rf_record_path: str = os.path.join(file_dir, f"predicted_record_{ModelName.rf}.txt")
    adabr_record_path: str = os.path.join(file_dir, f"predicted_record_{ModelName.adabr}.txt")

    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    save_dir: str = os.path.join(base_dir, "output")
    seq_model_name: str = f"{ModelName.anet}.pt"
    gbrt_model_name: str = f"{ModelName.gbrt}.pkl"
    rf_model_name: str = f"{ModelName.rf}.pkl"
    adabr_model_name: str = f"{ModelName.adabr}.pkl"
    # model saved path
    seq_model_path: str = os.path.join(save_dir, seq_model_name)
    gbrt_model_path: str = os.path.join(save_dir, gbrt_model_name)
    rf_model_path: str = os.path.join(save_dir, rf_model_name)
    adabr_model_path: str = os.path.join(save_dir, adabr_model_name)
