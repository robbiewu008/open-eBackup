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
import random
from typing import List

import torch

from app.common.schemas.job_schemas import JobSchema
import app.ai_sorter.model as model
import app.ai_sorter.tensor as tensor
import app.ai_sorter.utils as utils
from app.ai_sorter.service.train import TRAIN_WINDOW_SIZE
from app.common import logger

log = logger.get_logger(__name__)


def reschedule_jobs(job_list: List[JobSchema]) -> List[JobSchema]:
    # 先尝试reload之前的Model文件
    if not tensor.reload_final_model():
        log.info("Model is not ready.")

    return sort_jobs(job_list,
                     model.StaticAIModelContainer.ai_model,
                     model.StaticAIModelContainer.norm_paras,
                     model.StaticAIModelContainer.retrain_time)


def sort_jobs(job_list: List[JobSchema],
              ai_model: model.LSTM,
              norma_paras,
              retrain_time) -> List[JobSchema]:
    queue = model.PriorityQueue(tensor.opt.schedule_policy,
                                tensor.opt.mlq_priorities)
    job_map = {}

    for job in job_list:
        vm_item = utils.parse_vm_from_job(job)
        job_map[vm_item.job_id] = job

        prev_job_speed_list = utils.get_prev_job_speed_list(vm_item,
                                                            TRAIN_WINDOW_SIZE)
        if len(prev_job_speed_list) == 0:
            vm_item.predict_easy_degree = torch.zeros(1, 1, 1) + random.random()
        else:
            vm_item.previous_easy_degree = torch.Tensor(
                prev_job_speed_list).reshape(1, -1, 1)
            vm_item.predict_easy_degree = \
                vm_item.previous_easy_degree[:, -1].unsqueeze(1)
        if ai_model is not None and \
                retrain_time > vm_item.first_online_time and \
                len(prev_job_speed_list) >= 2:
            # 使用模型预测vm_item的job_speed
            vm_item.predict_easy_degree = \
                predict_job_speed_use_ai(ai_model, norma_paras,
                                         vm_item.previous_easy_degree)
        queue.queue_push(vm_item)

    sorted_job_list = []
    for _ in range(len(job_list)):
        vm_item = queue.queue_pop()
        new_job = job_map.get(vm_item.job_id)
        sorted_job_list.append(new_job)
    return sorted_job_list


def predict_job_speed_use_ai(ai_model: model.LSTM,
                             norm_paras,
                             previous_easy_degree):
    """
    通过AI模型预测当前任务的难易程度，并更新vm_item.predict_easy_degree
    :param ai_model: AI模型
    :param norm_paras: 归一化参数
    :param previous_easy_degree: job数据
    :return 使用AI模型预测到的vm_item的job速度
    """
    try:
        y_pred, _ = ai_model(
            (previous_easy_degree - norm_paras[0]) / norm_paras[1])
    except ZeroDivisionError:
        y_pred, _ = ai_model(previous_easy_degree - norm_paras[0])
        y_pred += norm_paras[0]
    else:
        y_pred = y_pred * norm_paras[1] + norm_paras[0]
    y_pred = y_pred[0, -1, 0]
    y_pred = min(y_pred, previous_easy_degree.max() * 10)
    y_pred = max(y_pred, torch.zeros_like(y_pred) + 0.0001)
    return y_pred.squeeze()
