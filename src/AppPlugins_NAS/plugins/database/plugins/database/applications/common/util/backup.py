#
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
#

import functools
from threading import Thread
from common.util.backup_utils import Backup, BackupStatus

BACKUP_TASK = dict()
BACKUP_THREAD_NUM = 256
BACKUP_MAX_MEMORY = 104857600


def async_call(func):
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        Thread(target=func, args=args, kwargs=kwargs).start()
        return True
    return wrapper


@async_call
def backup(job_id, source, destination, write_meta=True, thread_num=BACKUP_THREAD_NUM, max_memory=BACKUP_MAX_MEMORY):
    """
    备份目录（异步）
    :param: source:源目录，destination:目标路径，write_meta:是否备份元数据，thread_num:线程数，max_memory:内存大小
    :return: True:成功，False:失败
    """
    global BACKUP_TASK
    inst = Backup(job_id, write_meta, thread_num, max_memory)
    if job_id in BACKUP_TASK:
        return True
    BACKUP_TASK[job_id] = inst
    dirs = [source]
    return inst.backup(dirs, destination)


@async_call
def backup_dirs(job_id, dirs, destination, write_meta=True, thread_num=BACKUP_THREAD_NUM, max_memory=BACKUP_MAX_MEMORY):
    """
    备份目录列表（异步）--所有目录必须是相同的父目录
    :param: source:源目录列表，destination:目标路径，write_meta:是否备份元数据，thread_num:线程数，max_memory:内存大小
    :return: True:成功，False:失败
    """
    global BACKUP_TASK
    inst = Backup(job_id, write_meta, thread_num, max_memory)
    if job_id in BACKUP_TASK:
        return True
    BACKUP_TASK[job_id] = inst
    return inst.backup(dirs, destination)


@async_call
def backup_files(job_id, files, destination, write_meta=False, write_queue_size=1000,
                 thread_num=BACKUP_THREAD_NUM, max_memory=BACKUP_MAX_MEMORY):
    """
    备份文件列表（异步） --所有文件必须是相同的父目录
    :param: files:源文件列表，destination:目标路径，thread_num:线程数，max_memory:内存大小
    :return: True:成功，False:失败
    """
    global BACKUP_TASK
    inst = Backup(job_id, write_meta, thread_num, max_memory)
    if job_id in BACKUP_TASK:
        return True
    BACKUP_TASK[job_id] = inst
    return inst.backup(files, destination, write_queue_size)


def query_progress(job_id):
    """
    查询备份进度
    :param:
    :return: status:状态，progress:进度，dataSize:已备份的数据量（KB）
    """
    progress = 0
    data_size = 0
    if job_id not in BACKUP_TASK:
        return BackupStatus.BACKUP_FAILED.value, progress, data_size
    inst = BACKUP_TASK.get(job_id)
    status, progress, data_size = inst.query_backup_progress()
    if status != BackupStatus.BACKUP_INPROGRESS.value:
        del BACKUP_TASK[job_id]
    return status, progress, data_size
