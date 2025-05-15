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

import pandas as pd
import torch
from torch.utils.data import TensorDataset
from app.smart_balance.schemas import option, smconst
from app.common import logger
from app.smart_balance.utils.dataset_utils import dimensional_normalization

log = logger.get_logger(__name__)


def preprocessing_for_seq(input_type, input_size, input_time):
    input_type = torch.Tensor(input_type)
    input_time = torch.Tensor(input_time)
    last_time_scale = torch.index_select(input_time, 1,
                                         torch.LongTensor([i for i in range(0, option.window_len * 2, 2)]))
    time_scale_length = torch.index_select(input_time, 1,
                                           torch.LongTensor([i + 1 for i in range(0, option.window_len * 2, 2)]))
    input_size = torch.Tensor(input_size)
    last_size_scale = torch.index_select(input_size, 1,
                                         torch.LongTensor([i for i in range(0, option.window_len * 2, 2)]))
    size_scale_length = torch.index_select(input_size, 1,
                                           torch.LongTensor([i + 1 for i in range(0, option.window_len * 2, 2)]))
    normed_dataset = torch.stack(
        [input_type, last_size_scale, size_scale_length, last_time_scale, time_scale_length]).permute(1, 2, 0).reshape(
        -1,
        option.window_len * smconst.flen)

    if normed_dataset.size(0) < smconst.epoch_rounds:
        train_set = TensorDataset(normed_dataset[:, :-smconst.flen], normed_dataset[:, -1])
        validate_set = train_set
    else:
        # shuffle the dataset
        train_size = int(normed_dataset.size(0) * 0.8)
        perm = torch.randperm(len(normed_dataset))

        train_set = TensorDataset(normed_dataset[perm][:train_size, :-smconst.flen],
                                  normed_dataset[perm][:train_size, -1])
        validate_set = TensorDataset(normed_dataset[perm][train_size:, :-smconst.flen],
                                     normed_dataset[perm][train_size:, -1])

    return [train_set, validate_set]


def read_for_preprocessing_for_ml(index, lines, line, data):
    for n_index in range(index + 1, len(lines)):
        next_line = lines[n_index].split(" ")
        if next_line[0] == line[0] and next_line[1] == line[1]:
            last_time_scale, time_scale_length = dimensional_normalization(line[2])
            last_size_scale, size_scale_length = dimensional_normalization(line[3])
            data.append(
                [float(line[1]), last_time_scale, time_scale_length, last_size_scale, size_scale_length])
            next_data_amount = next_line[-1]
            if "\n" in next_data_amount:
                next_data_amount = next_data_amount[:-1]
            data[-1].append(float(next_data_amount))
            break


def preprocessing_for_ml():
    """
    preprocess raw data like the following inputs\output format for ml model
    # model input: last_type, last_time_scale, time_scale_len, last_size_scale, size_scale_length
    # model output: cur_task_data_amount
    @return: dealed ml dataset
    """
    log.info("Start read txt for train ml")
    if not os.path.exists(option.dataset_path):
        return [], []
    data = []

    txt_file = open(option.dataset_path, "r", encoding="utf-8")
    lines = txt_file.readlines()
    for index, line in enumerate(lines):
        if "sourceName" in line:
            continue
        line = line[:-1].split(" ")
        if float(line[2]) < 0 or float(line[3]) < 0:
            continue
        read_for_preprocessing_for_ml(index, lines, line, data)
    txt_file.close()
    df = pd.DataFrame(data, columns=None)
    features = df.iloc[:, :-1]
    labels = df.iloc[:, -1]
    return features, labels


def seq_predict(previous_data, ai_model):
    # previous_data: type, [scale_size, scale_size_len], [scale_time, scale_time]
    if not previous_data[0]:
        log.info(f"No history, can not predict by ANet model")
        return -1
    input_type = torch.Tensor(previous_data[0])
    input_size = torch.Tensor(previous_data[1])
    last_size_scale = torch.index_select(input_size, 0,
                                         torch.LongTensor([i for i in range(0, (option.window_len - 1) * 2, 2)]))
    size_scale_length = torch.index_select(input_size, 0,
                                           torch.LongTensor([i + 1 for i in range(0, (option.window_len - 1) * 2, 2)]))
    input_time = torch.Tensor(previous_data[2])
    last_time_scale = torch.index_select(input_time, 0,
                                         torch.LongTensor([i for i in range(0, (option.window_len - 1) * 2, 2)]))
    time_scale_length = torch.index_select(input_time, 0,
                                           torch.LongTensor([i + 1 for i in range(0, (option.window_len - 1) * 2, 2)]))
    normed_dataset = torch.stack(
        [input_type, last_size_scale, size_scale_length, last_time_scale, time_scale_length]).permute(0, 1).reshape(
        -1, (option.window_len - 1) * smconst.flen)

    y_pred = ai_model(normed_dataset)
    log.info(f"ANet predict: {y_pred.item()}")
    return y_pred.item()


def ml_predict(previous_data, model, ml_name):
    # previous_data: previous_type, previous_backup_time, previous_data_amount
    if previous_data[0] <= 0:
        log.info(f"No history, can not predict by {ml_name} model")
        return -1
    previous_ml_data = [previous_data[0]]
    previous_backup_time = dimensional_normalization(previous_data[1])
    previous_data_amount = dimensional_normalization(previous_data[2])
    previous_ml_data.extend(previous_backup_time)
    previous_ml_data.extend(previous_data_amount)
    result = model.predict([previous_ml_data])
    log.info(f"{ml_name} model predict: {result}")
    return result[0]
