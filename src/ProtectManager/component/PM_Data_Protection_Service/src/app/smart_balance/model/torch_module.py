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
import torch.nn as nn
import torch


class ANet(torch.nn.Module):
    def __init__(self, drop_out=0.05, input_size=27, output_size=1):
        super(ANet, self).__init__()
        self.drop_out = drop_out
        self.embed_dim1 = 64
        self.embed_dim2 = 32
        self.output_size = output_size
        self.input_size = input_size
        self.best_loss = 100
        self.best_cnt = 0
        self.multihead_attn = nn.MultiheadAttention(num_heads=8, dropout=self.drop_out, embed_dim=self.embed_dim1)
        self.pre_node = nn.Sequential(
            nn.Linear(self.input_size, self.embed_dim1),
            nn.ReLU(),
            nn.Linear(self.embed_dim1, self.embed_dim1),
        )
        self.linear = nn.Sequential(
            nn.Linear(self.embed_dim1, self.embed_dim2),
            nn.ReLU(),
            nn.Dropout(drop_out),
            nn.Linear(self.embed_dim2, self.output_size),

        )
        for param in self.parameters():
            if len(param.shape) > 1:
                nn.init.xavier_uniform_(param)

    def forward(self, features):
        # query: :math:`(L, N, E)` where L is the target sequence length, N is the batch size, E is
        #           the embedding dimension.

        batch_size = features.size(0)
        tmp = self.pre_node(features)
        tmp = tmp.view(-1, batch_size, self.embed_dim1)
        tmp, _ = self.multihead_attn(tmp, tmp, tmp)
        tmp = self.linear(tmp).squeeze()
        return tmp
