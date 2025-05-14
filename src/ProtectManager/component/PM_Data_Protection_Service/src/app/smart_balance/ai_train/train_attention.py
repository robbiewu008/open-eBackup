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
import torch
import torch.nn.functional as F

from app.smart_balance.model import ANet
from app.smart_balance.schemas import option, smconst
from app.common import logger

log = logger.get_logger(__name__)

device = torch.device('cpu')


def run_attention(train_set, validate_set):
    # run and train the AI model
    # input：  [job_type，size_of_job, finished_time_of_job] * window_len
    # output： size of the backup job

    train_model = ANet(drop_out=option.drop_out, input_size=(option.window_len - 1) * smconst.flen,
                       output_size=option.output_size)
    optim = torch.optim.AdamW(train_model.parameters(), lr=option.lr)
    os.makedirs(option.save_dir, exist_ok=True)

    for epoch in range(option.n_epochs):
        train_model.train()
        train_loss = train_trainset(train_model, optim, train_set)

        train_model.eval()
        # fit in the validate_dateset
        validate_loss = train_validateset(train_model, validate_set)
        log.info("Epoch: {} train_loss is {}, validate_loss is {}".format(epoch, train_loss, validate_loss))

        if validate_loss < train_model.best_loss and epoch > smconst.epoch_rounds:
            train_model.best_loss = validate_loss
            train_model.best_cnt = 0
            torch.save(
                {
                    'model': train_model.state_dict(),
                    'optimizer': optim.state_dict(),
                    'epoch': epoch
                },
                option.seq_model_path)
            log.info("Epoch: {} best_reward is {}".format(epoch, train_model.best_loss))
        else:
            train_model.best_cnt += 1

        if train_model.best_cnt > option.n_patients and epoch > smconst.epoch_rounds:
            log.info(f"Tran epoch {epoch}, best_loss: {train_model.best_loss}")
            break


def train_trainset(model, optim, train_loader):
    # AI model fit the train_dateset
    loss_record = []
    for _, item in enumerate(train_loader):
        features, lable = item
        lable = lable.to(device).float()
        features = features.to(device)
        out = model(features)
        out = torch.squeeze(out)
        loss = F.mse_loss(out, lable)
        optim.zero_grad()
        loss.backward()
        torch.nn.utils.clip_grad_norm_(model.parameters(), max_norm=20, norm_type=2)
        optim.step()
        loss_record.append(loss.item())
    return torch.tensor(loss_record).mean().item()


def train_validateset(model, test_loader):
    # AI model fit the validate_dateset
    loss_record = []
    for _, item in enumerate(test_loader):
        features, lable = item
        lable = lable.to(device).float()
        features = features.to(device)
        out = model(features)
        lable = lable.to(device).float()
        loss = F.mse_loss(out, lable)
        loss_record.append(loss.item())

    return torch.tensor(loss_record).mean().item()
