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
import torch
import app.ai_sorter.tensor.config as config
import app.ai_sorter.model as model


def train(ai_model: model.LSTM,
          optimizer: torch.optim.Adam,
          training_dataloader, validate_dataloader,
          opts: config.Options):
    """
    after processed dataset, start to train model
    :param ai_model: AI model
    :param optimizer: optimizer
    :param training_dataloader: train set's dataloader
    :param validate_dataloader: validate set's dataloader
    :param opts: AI model's configuration
    :return:
    """

    for _ in range(opts.n_epochs):
        is_early_stop = train_epoch(ai_model, training_dataloader,
                                    validate_dataloader, optimizer, opts)
        # if loss do not reduce, stop the training
        if is_early_stop:
            break
    return


def train_epoch(ai_model: model.LSTM,
                training_dataloader, validate_dataloader,
                optimizer: torch.optim.Adam,
                opts: config.Options):
    """
    training step
    :param ai_model: AI model
    :param optimizer: optimizer
    :param training_dataloader: train set's dataloader
    :param validate_dataloader: validate set's dataloader
    :param opts: AI model's configuration
    :return:
    """
    # training step
    ai_model.train()
    for _, batch in enumerate(training_dataloader):
        y_pred, _ = ai_model(batch[0].unsqueeze(-1))
        loss = opts.loss_function(y_pred[:, :, 0], batch[1])
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

    # validate step
    ai_model.eval()
    val_loss_list = []
    for _, batch in enumerate(validate_dataloader):
        y_pred, _ = ai_model(batch[0].unsqueeze(-1).to(opts.device))
        loss = opts.loss_function(y_pred[:, :, 0].cpu(), batch[1]).unsqueeze(-1)
        val_loss_list.append(loss)

    # calculate mean loss of validate set
    val_loss = torch.cat(tuple(val_loss_list), 0).mean()

    if val_loss < ai_model.best_loss:
        ai_model.best_loss = val_loss
        ai_model.best_cnt = 0
    else:
        ai_model.best_cnt += 1

    is_early_stop = False
    if ai_model.best_cnt > opts.n_patients:
        is_early_stop = True
    return is_early_stop
