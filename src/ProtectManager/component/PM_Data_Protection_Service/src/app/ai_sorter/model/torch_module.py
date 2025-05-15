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


class LSTM(nn.Module):
    def __init__(self, input_size=1, output_size=1, hidden_layer_size=128,
                 lstm_layers=3):
        super().__init__()

        self.hidden_layer_size = hidden_layer_size
        self.lstm = nn.LSTM(input_size, hidden_layer_size, batch_first=True,
                            num_layers=lstm_layers)
        self.linear = nn.Linear(hidden_layer_size, output_size)
        self.best_loss = 1e9
        self.best_cnt = 0

    def forward(self, input_seq, cell_state=None):
        if cell_state is not None:
            cell_state = (cell_state[0].detach(), cell_state[0].detach())
            lstm_out, cell_state = self.lstm(input_seq, cell_state)
        else:
            lstm_out, cell_state = self.lstm(input_seq)
        predictions = self.linear(lstm_out)
        return predictions, cell_state
