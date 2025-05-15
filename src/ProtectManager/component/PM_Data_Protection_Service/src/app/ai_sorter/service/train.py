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
from typing import List

import app.ai_sorter.utils as utils
import app.ai_sorter.tensor as tensor

import app.ai_sorter.model as model
from app.common.schemas.job_schemas import JobSchema
from app.protection.object.models.projected_object import ProtectedObject

from app.common import logger

log = logger.get_logger(__name__)

TRAIN_WINDOW_SIZE = 10


def train_start():
    """
    创建线程异步执行训练任务

    :return: none
    """
    train_thread = threading.Thread(target=train_model)
    try:
        train_thread.start()
    except threading.ThreadError as th_err:
        log.error(f"Start train_model thread failed: ", {th_err})


def train_model():
    """
    训练AI模型

    :return: 训练集
    """
    if model.StaticAIModelContainer.training_lock.acquire(False):
        try:
            vmware_protect_objs = utils.get_vmware_objects()
            # 准备job_speed_set数据集
            job_speed_set = prepare_data_set(vmware_protect_objs)

            # job_speed数据集 -> tensor训练集 + tensor验证集 + 归一化参数
            train_set, validate_set, norm_paras = tensor.preprocessing(
                job_speed_set, TRAIN_WINDOW_SIZE)

            # tensor验证集 + tensor训练集 + 归一化参数 -> 训练 -> aimodel文件
            tensor.train(train_set, validate_set, norm_paras)
        finally:
            model.StaticAIModelContainer.training_lock.release()
    else:
        log.info(f"There is another training job in processing")


def prepare_data_set(project_objs: List[ProtectedObject]):
    """
    根据保护资源准备训练数据集
    1. 迭代取各个保护资源中取出连续成功的备份成功任务列表（大小: 10 - 200)。
    2. 将成功任务的难易程度（备份速率）加入数据集
    3. 将所有的数据集合并。

    :param project_objs: 保护资源
    :return: 数据集
    """
    data_set = []
    for protect_obj in project_objs:
        job_list = utils.get_backup_jobs_per_vm(protect_obj,
                                                TRAIN_WINDOW_SIZE)
        if len(job_list) == 0:
            continue
        add_new_data_in_data_set(job_list, data_set)

    return data_set


def add_new_data_in_data_set(job_list: List[JobSchema], data_set):
    """
    准备训练所使用的数据集
    :param job_list: 某一个保护资源的历史完成任务
    :param data_set: 待放入训练数据的dataset列表， dataset中的每个元素为job速率：MB/s
    :return: None
    """
    part_of_dataset = []
    for job_inf in job_list:
        job_speed = utils.get_job_speed(job_inf)
        # 如果job size信息不正常，则continue
        if job_speed < 0:
            # 因为训练需要连续时序jobs数据，如果有异常数据，
            # 则将有用的数据保留，并清空part_of_data，重新开始收集连续数据
            data_set += get_continuous_sequences(part_of_dataset)
            part_of_dataset = []
            continue
        # 将容易程度放入part_of_dataset
        part_of_dataset.append(job_speed)

    # 去除连续序列的冗余数据(超出n*train_window的部分)
    data_set += get_continuous_sequences(part_of_dataset)


def get_continuous_sequences(part_of_dataset):
    """
    从输入的part_of_dataset中，获取连续序列，即：train_window的倍数长度
    :param part_of_dataset: 待获取连续序列的部分dataset
    :return: 返回去除冗余后的训练数据集
    """
    # 如果这部分dataset数量小于训练窗口大小，返回空list，否则返回train_window的倍数长度的数据
    if len(part_of_dataset) < TRAIN_WINDOW_SIZE:
        return []
    redundancy = len(part_of_dataset) % TRAIN_WINDOW_SIZE
    return part_of_dataset[redundancy:]
