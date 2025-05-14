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
from datetime import datetime

import torch
from torch.utils.data import DataLoader, TensorDataset
import app.ai_sorter.model as model
import app.ai_sorter.tensor.flow_neural_network as flow_neural_network
import app.ai_sorter.tensor.config as config

from app.common import logger

log = logger.get_logger(__name__)


def reload_final_model() -> bool:
    """
    重新加载的Model文件

    :return: 成功或失败
    """
    # ai_model变量已经有值，说明不需要重新训练
    if model.StaticAIModelContainer.ai_model:
        return True

    if not os.path.isfile(config.opt.final_model_path):
        return False

    if model.StaticAIModelContainer.model_load_lock.acquire(False):
        try:
            model_params_map = torch.load(config.opt.final_model_path)
            if 'model' not in model_params_map or \
                    'retrain_time' not in model_params_map or \
                    'norm_paras' not in model_params_map:
                return False
            model.StaticAIModelContainer.ai_model = model_params_map['model']
            model.StaticAIModelContainer.retrain_time = \
                model_params_map['retrain_time']
            model.StaticAIModelContainer.norm_paras = \
                model_params_map['norm_paras']
            return True
        finally:
            model.StaticAIModelContainer.model_load_lock.release()
    else:
        log.info(f"Other thread is loading the Model now.")
        return False


def preprocessing(dataset, train_window):
    """
    数据预处理，拆分训练集和验证集，计算归一化参数
    :param dataset: 从历史任务抽取的dataset
    :param train_window: 训练窗口大小
    :return: 训练集、测试集以及归一化参数
    """
    tensor_set = torch.Tensor(dataset).reshape(-1, train_window)
    data_mean = tensor_set[tensor_set > -1].mean().item()
    data_std = tensor_set[tensor_set > -1].std().item()
    norm_paras = [data_mean, data_std]
    try:
        normed_dataset = (tensor_set - data_mean) / data_std
    except ZeroDivisionError:
        log.info("data_std is zero")
        norm_paras[1] = 1
        normed_dataset = (tensor_set - data_mean)

    # 训练组数: len(dataset)/train_window如果小于10，则全都作为训练集，否则训练取80%、验证集取20%
    if normed_dataset.size(0) < 10:
        train_set = TensorDataset(normed_dataset[:, :-1], normed_dataset[:, 1:])
        validate_set = train_set
    else:
        train_size = int(normed_dataset.size(0) * 0.8)
        train_set = TensorDataset(normed_dataset[:train_size, :-1],
                                  normed_dataset[:train_size, 1:])
        validate_set = TensorDataset(normed_dataset[train_size:, :-1],
                                     normed_dataset[train_size:, 1:])

    return train_set, validate_set, norm_paras


def train(train_set, validate_set, norm_paras):
    """
    数据集整理完毕之后，正式开始训练模型
    :param train_set: 训练集
    :param validate_set: 验证集
    :param norm_paras: 归一化参数
    :return: 无返回
    """
    best_ai_model_loss = 1e9
    for batch_sz in config.opt.batch_sizes:
        for hidden_size in config.opt.hidden_sizes:
            for lstm_layer in config.opt.lstm_layers:
                ai_model = model.LSTM(hidden_layer_size=hidden_size,
                                      lstm_layers=lstm_layer)
                optimizer = torch.optim.Adam(
                    [{'params': ai_model.parameters(),
                      'lr': config.opt.lr_model}])
                training_dataloader = \
                    torch.utils.data.DataLoader(train_set,
                                                batch_size=batch_sz,
                                                shuffle=True,
                                                num_workers=0)
                validate_dataloader = \
                    torch.utils.data.DataLoader(validate_set,
                                                batch_size=batch_sz,
                                                shuffle=True,
                                                num_workers=0)
                flow_neural_network.train(ai_model, optimizer,
                                          training_dataloader,
                                          validate_dataloader,
                                          config.opt)
                log.info(
                    f"training over, current loss: {ai_model.best_loss},"
                    f" best loss: {best_ai_model_loss}")
                best_ai_model_loss = update_tmp_model_file(ai_model,
                                                           best_ai_model_loss,
                                                           norm_paras)
    update_final_model_file()


def update_tmp_model_file(ai_model: model.LSTM, best_loss, norm_paras):
    """
    比较ai_model中的best_loss和当前best_loss，如果ai_model中的best_loss更小，则更新文件

    :param ai_model:
    :param best_loss: 当前best_loss
    :param norm_paras: 训练集的归一化参数
    :return: 更新后的best_loss
    """
    if ai_model.best_loss > best_loss:
        return best_loss
    time_now = datetime.utcnow().timestamp()
    os.makedirs(config.opt.save_dir, exist_ok=True)

    torch.save(
        {
            'model': ai_model,
            'retrain_time': time_now,
            'norm_paras': norm_paras
        },
        config.opt.tmp_model_path
    )
    return ai_model.best_loss


def update_final_model_file():
    """
    使用训练后产生的更新的model文件去刷新最终model文件
    bestModelTrain.pt -> bestModel.pt

    :return: None
    """

    if not os.path.isfile(config.opt.tmp_model_path):
        log.warning(f"Train failed this time")
        return

    # 使用更新tmp_model_path 替代 final_model_path
    if os.path.isfile(config.opt.final_model_path):
        os.remove(config.opt.final_model_path)
    os.rename(config.opt.tmp_model_path, config.opt.final_model_path)
    model.StaticAIModelContainer.ai_model = None
