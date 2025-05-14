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

import joblib
import torch
from app.smart_balance.schemas import option, smconst
from app.common import logger
from app.smart_balance.model import ANet, StaticContainer

log = logger.get_logger(__name__)


def load_model():
    # check the need for reloading model
    has_built_model = StaticContainer.seq_model and StaticContainer.gbrt_model \
                      and StaticContainer.rf_model and StaticContainer.ada_model
    if has_built_model:
        return True

    if StaticContainer.model_load_lock.acquire(False):
        try:
            trained_model_count = 0
            if os.path.isfile(option.gbrt_model_path):
                StaticContainer.gbrt_model = joblib.load(option.gbrt_model_path)
                trained_model_count += 1
                log.info("GBRT model is ready.")
            if os.path.isfile(option.rf_model_path):
                StaticContainer.rf_model = joblib.load(option.rf_model_path)
                trained_model_count += 1
                log.info("RF model is ready.")
            if os.path.isfile(option.adabr_model_path):
                StaticContainer.ada_model = joblib.load(option.adabr_model_path)
                trained_model_count += 1
                log.info("AdaBR model is ready.")
            if os.path.isfile(option.seq_model_path):
                model_params_map = torch.load(option.seq_model_path)
                if 'model' in model_params_map:
                    seq_model = ANet(input_size=(option.window_len - 1) * smconst.flen, output_size=option.output_size)
                    seq_model.load_state_dict(model_params_map['model'])
                    seq_model.eval()
                    StaticContainer.seq_model = seq_model
                    trained_model_count += 1
                    log.info("ANet model is ready.")
            if trained_model_count >= 1:
                return True
            return False
        finally:
            StaticContainer.model_load_lock.release()
    log.info(f"Other thread is loading the Model now.")
    return False
