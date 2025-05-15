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
import threading

import torch
from app.smart_balance.schemas import option, ModelName
import app.smart_balance.model as model
import app.smart_balance.utils as utils
from app.smart_balance.utils import preprocessing_for_ml
from app.smart_balance.ai_train import train_attention, train_ml
from app.common import logger

log = logger.get_logger(__name__)


def train_start():
    train_thread = threading.Thread(target=train_model)
    try:
        train_thread.start()
    except threading.ThreadError as th_err:
        log.error(f"Start train_model thread failed: ", {th_err})


def train_model():
    model.seed_torch()
    if model.StaticContainer.training_lock.acquire(False):
        # get previous raw data
        input_type, input_size, input_time = utils.read_txt_for_seq_train()
        if not input_type:
            log.info(f"No data for building AI model")
            return
        log.info("Get previous raw data success")
        # preprocess the data
        train_set, validate_set = utils.preprocessing_for_seq(input_type, input_size, input_time)
        log.info(f"Preprocess previous raw data success")
        log.info(f"The length of train set is {len(train_set)}, the length of validate set is {len(validate_set)}")
        if len(validate_set) < option.batch_sizes:
            log.info(f"Less data for building AI model")
            return
        training_dataloader = \
            torch.utils.data.DataLoader(train_set,
                                        batch_size=option.batch_sizes,
                                        shuffle=True,
                                        num_workers=0,
                                        drop_last=True)
        validate_dataloader = \
            torch.utils.data.DataLoader(validate_set,
                                        batch_size=option.batch_sizes,
                                        shuffle=False,
                                        num_workers=0,
                                        drop_last=True)
        # train the multi-head attention model
        train_attention.run_attention(training_dataloader, validate_dataloader)
        log.info(f"Build Attention model successful")
        features, labels = preprocessing_for_ml()
        log.info("Preprocess for ml finished")
        train_ml.train_ml_model(option.gbrt_model_path, features, labels, ModelName.gbrt)
        log.info(f"Build GBRT model successful")
        train_ml.train_ml_model(option.rf_model_path, features, labels, ModelName.rf)
        log.info(f"Build RF model successful")
        train_ml.train_ml_model(option.adabr_model_path, features, labels, ModelName.adabr)
        log.info(f"Build AdaBR model successful")
        model.StaticContainer.training_lock.release()
    else:
        log.info(f"There is another training job in processing")
