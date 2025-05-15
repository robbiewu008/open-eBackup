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
from typing import List
import torch

from app.common import logger

log = logger.get_logger(__name__)


class VM(object):
    """
    :param self.name: source name
    :param self.schedule_freq: job's schedule backup frequency
    :param self.priority: job's priority
    :param self.first_online_time: first backup time
    """

    def __init__(self, job_id, name, backup_freq, priority, first_online_time):
        self.job_id = job_id
        self.name = name
        self.schedule_freq = backup_freq
        self.priority = priority
        self.first_online_time = first_online_time
        self.previous_easy_degree = torch.zeros(1, 1, 1)
        self.predict_easy_degree = torch.Tensor([0])


class PriorityQueue(object):
    """
    :param self.mlq_priorities: Multi level queue scheduling priorities
    :param self.schedule_freq: sorter work type: AI or priority
    :param self.queues: queues for corresponding priority
    :param self.queue_len: len of queue
    """

    def __init__(self, schedule_policy, mlq_priorities):
        self.mlq_priorities = mlq_priorities
        self.schedule_policy = schedule_policy
        self.queues: List[List[VM]] =\
            [[] for _ in range(max(self.mlq_priorities) + 1)]
        self.queue_len = 0

    def queue_pop(self) -> VM:
        """
        pop the job from priority queue
        :@return: vm_item
        """
        vm_item = None
        queues_num = len(self.queues)
        for idx in range(len(self.queues)):  # highest priority departs first
            queue = self.queues[queues_num - idx - 1]
            if queue:
                self.queue_len -= 1
                if self.schedule_policy == 0:  # FlowNN based scheduling policy
                    return queue.pop(
                        -1)  # pop out the vm with largest jobsize at index=-1
                elif self.schedule_policy == 1:  # incumbent scheduling policy of A8000
                    return queue.pop(0)
        return vm_item  # return none if queue is empty

    def queue_push(self, new_vm: VM):
        """
        push the new_vm into corresponding priority queue
        根据predict_easy_degree, 在对应的优先级队列实现从小到大排序
        :@param new_vm: the vm object need to push
        :@return:
        """
        self.queue_len += 1
        for loc, vm_item in enumerate(self.queues[new_vm.priority]):
            if new_vm.predict_easy_degree < vm_item.predict_easy_degree:
                self.queues[new_vm.priority].insert(loc, new_vm)
                return
        self.queues[new_vm.priority].append(new_vm)
