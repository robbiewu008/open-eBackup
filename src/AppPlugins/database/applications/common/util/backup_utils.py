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

import os
import shutil
import time
from ctypes import cdll, Structure, c_char_p, c_void_p, c_int, c_uint64, POINTER, c_bool
from enum import Enum

from common.env_common import get_install_head_path
from common.logger import Logger
from common.util.scanner_utils import Scanner, get_log_level

LOGGER = Logger().get_logger()
_LIB_BACKUP_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/lib"
_SCANNER_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/scantmp"


class BackupStatistics(Structure):
    _fields_ = [
        ("noOfDirToBackup", c_uint64),        # Number of directories to be backed up
        ("noOfFilesToBackup", c_uint64),      # Number of files to be backed up
        ("noOfBytesToBackup", c_uint64),      # Number of bytes (in KB) to be backed up
        ("noOfDirCopied", c_uint64),          # Number of directories copied
        ("noOfFilesCopied", c_uint64),        # Number of files copied
        ("noOfBytesCopied", c_uint64),        # Number of bytes (in KB) copied
        ("noOfDirFailed", c_uint64),          # Number of directories copy failed
        ("noOfFilesFailed", c_uint64),        # Number of files copy failed
        ("backupspeed", c_uint64)             # Backup speed (in KBps)
    ]


class BackupType(int, Enum):
    PHASE_COPY = 1
    PHASE_DELETE = 2
    PHASE_HARDLINK = 3
    PHASE_DIR = 4


class BackupStatus(int, Enum):
    BACKUP_COMPLETED = 1
    BACKUP_INPROGRESS = 2
    BACKUP_FAILED = 3


class BackupStep(int, Enum):
    BACKUP_PREPARE = 1
    BACKUP_COPY = 2
    BACKUP_HARDLINK = 3
    BACKUP_DIR = 4


class Backup(object):
    def __init__(self, job_id, write_meta, thread_num, max_memory):
        self._job_id = job_id
        self._copy_inst = None
        self._hardlink_inst = None
        self._dir_inst = None
        self._step = BackupStep.BACKUP_PREPARE.value
        lib_path = os.path.realpath(os.path.join(_LIB_BACKUP_PATH, "libBackup.so"))
        self._backup = cdll.LoadLibrary(lib_path)
        self._backup.InitLog.argtypes = [c_char_p, c_int]
        log_path = os.getenv("GENERALDB_LOG_PATH")
        log_level = get_log_level()
        self._backup.InitLog(log_path.encode(), log_level)
        self._meta_path = os.path.realpath(os.path.join(_SCANNER_PATH, job_id, "meta"))
        self._meta_ctl_path = os.path.realpath(os.path.join(_SCANNER_PATH, job_id, "meta_ctl"))
        self._write_meta = write_meta
        self._thread_num = thread_num
        self._max_memory = max_memory

    def __del__(self):
        self._destroy(self._copy_inst)
        self._destroy(self._hardlink_inst)
        self._destroy(self._dir_inst)

    def backup(self, dirs, destination, write_queue_size=1000):
        if not dirs:
            return False
        os.umask(0o000)
        try:
            if not self._prepare_files(dirs, write_queue_size):
                return False
            source = dirs[0]
            if not self._copy(source, destination):
                return False
            if self._hardlink_exists() and not self._hardlink(source, destination):
                return False
            if not self._chmeta(source, destination):
                return False
        finally:
            self._clean_scanner_path()
            os.umask(0o027)
        return True

    def query_backup_progress(self):
        data_size = 0
        progress = 0
        LOGGER.info(f"Backup step:{self._step}, jobId:{self._job_id}")
        if self._step == BackupStep.BACKUP_PREPARE.value:
            return BackupStatus.BACKUP_INPROGRESS.value, progress, data_size
        elif self._step == BackupStep.BACKUP_COPY.value:
            status, progress, data_size = self._query_progress(self._copy_inst)
            status = status if status == BackupStatus.BACKUP_FAILED else BackupStatus.BACKUP_INPROGRESS.value
            return status, progress, data_size
        elif self._step == BackupStep.BACKUP_HARDLINK.value:
            status, progress, data_size = self._query_progress(self._hardlink_inst)
            status = status if status == BackupStatus.BACKUP_FAILED else BackupStatus.BACKUP_INPROGRESS.value
            return status, progress, data_size
        else:
            if not self._write_meta:
                return BackupStatus.BACKUP_COMPLETED, progress, data_size
            return self._query_progress(self._dir_inst)

    def _clean_scanner_path(self):
        LOGGER.info(f"Begin to clean scanner path, jobId:{self._job_id}")
        backup_scan_path = os.path.realpath(os.path.join(_SCANNER_PATH, self._job_id))
        if os.path.exists(backup_scan_path):
            shutil.rmtree(backup_scan_path)

    def _copy(self, source, destination):
        LOGGER.info(f"Backup file from {source} to {destination}, jobId:{self._job_id}")
        source_dir_path = os.path.dirname(source)
        backup_ctl_files = self._get_control_file(BackupType.PHASE_COPY.value)
        for ctl_file in backup_ctl_files:
            self._copy_inst = self._create(source_dir_path, destination, BackupType.PHASE_COPY.value)
            if not self._copy_inst:
                return False
            self._enqueue(self._copy_inst, ctl_file)
            self._start(self._copy_inst)
            self._step = BackupStep.BACKUP_COPY.value
            if not self._monitor(self._copy_inst):
                return False
        return True

    def _hardlink_exists(self):
        for file in os.listdir(self._meta_ctl_path):
            if file.startswith('hardlink'):
                return True
        return False

    def _hardlink(self, source, destination):
        source_dir_path = os.path.dirname(source)
        backup_ctl_file = self._get_control_file(BackupType.PHASE_HARDLINK.value)
        for ctl_file in backup_ctl_file:
            self._hardlink_inst = self._create(source_dir_path, destination, BackupType.PHASE_HARDLINK.value)
            if not self._hardlink_inst:
                return False
            self._enqueue(self._hardlink_inst, ctl_file)
            self._start(self._hardlink_inst)
            self._step = BackupStep.BACKUP_HARDLINK.value
            if not self._monitor(self._hardlink_inst):
                return False
        return True

    def _chmeta(self, source, destination):
        LOGGER.info(f"Begin to chmeta, jobId:{self._job_id}")
        if not self._write_meta:
            self._step = BackupStep.BACKUP_DIR.value
            return True
        source_dir_path = os.path.dirname(source)
        self._dir_inst = self._create(source_dir_path, destination, BackupType.PHASE_DIR.value)
        if not self._dir_inst:
            return False
        backup_ctl_file = self._get_control_file(BackupType.PHASE_DIR.value)
        for ctl_file in backup_ctl_file:
            self._enqueue(self._dir_inst, ctl_file)
        self._start(self._dir_inst)
        self._step = BackupStep.BACKUP_DIR.value
        LOGGER.info(f"End to chmeta, jobId:{self._job_id}")
        return self._monitor(self._dir_inst)

    def _backup_files_post(self, files, destination):
        for file in files:
            file_path = destination + file
            shutil.move(file_path, destination)
        for key in os.listdir(destination):
            tmp_path = os.path.realpath(os.path.join(destination, key))
            if os.path.isdir(tmp_path):
                shutil.rmtree(tmp_path)
        self._step = BackupStep.BACKUP_DIR.value
        return True

    def _monitor(self, instance):
        while True:
            status = self._get_status(instance)
            if status == BackupStatus.BACKUP_COMPLETED.value:
                break
            elif status == BackupStatus.BACKUP_FAILED.value:
                return False
            time.sleep(2)
        return True

    def _query_progress(self, instance):
        LOGGER.info(f"Begin to query process, jobId:{self._job_id}")
        progress = 0
        stats = BackupStatistics()
        status = self._get_status(instance)
        self._get_stats(instance, stats)
        dir_failed_cnt = stats.noOfDirFailed
        bytes_to_backup = stats.noOfBytesToBackup
        bytes_copied = stats.noOfBytesCopied
        data_size = int(stats.noOfBytesCopied / 1024)
        if dir_failed_cnt != 0:
            LOGGER.info(
                f"Backup failed, status:{status}, progress:{progress}, data_size:{data_size}, jobId:{self._job_id}")
            return BackupStatus.BACKUP_FAILED.value, progress, data_size
        if bytes_to_backup != 0:
            progress = int(bytes_copied * 100 / bytes_to_backup)
        LOGGER.info(f"Backup status:{status}, progress:{progress}, data_size:{data_size}, jobId:{self._job_id}")
        return status, progress, data_size

    def _get_control_file(self, phase):
        control_files = []
        backup_type_phase_map = {
            BackupType.PHASE_COPY.value: 'control',
            BackupType.PHASE_DELETE.value: 'delete',
            BackupType.PHASE_HARDLINK.value: 'hardlink',
            BackupType.PHASE_DIR.value: 'mtime'
        }
        key = backup_type_phase_map.get(phase, None)
        for file in os.listdir(self._meta_ctl_path):
            if file.startswith(key):
                control_files.append(os.path.realpath(os.path.join(self._meta_ctl_path, file)))
        return control_files

    def _prepare(self, backup_dir):
        scanner = Scanner()
        if not os.path.exists(_SCANNER_PATH):
            os.makedirs(_SCANNER_PATH)
        if not os.path.exists(self._meta_path):
            os.makedirs(self._meta_path)
        if not os.path.exists(self._meta_ctl_path):
            os.makedirs(self._meta_ctl_path)
        scanner.create(self._job_id, self._meta_path, self._meta_ctl_path)
        scanner.start(backup_dir)
        return scanner.monitor(self._job_id)

    def _prepare_files(self, files, write_queue_size=1000):
        LOGGER.info(f"Prepare files begin, jobId:{self._job_id}")
        if not os.path.exists(_SCANNER_PATH):
            os.makedirs(_SCANNER_PATH)
        if not os.path.exists(self._meta_path):
            os.makedirs(self._meta_path)
        if not os.path.exists(self._meta_ctl_path):
            os.makedirs(self._meta_ctl_path)
        scanner = Scanner()
        scanner.create(self._job_id, self._meta_path, self._meta_ctl_path, True, write_queue_size)
        scanner.start_scan_files(files)
        LOGGER.info(f"Prepare files end, jobId:{self._job_id}")
        return scanner.monitor(self._job_id)

    def _create(self, source, destination, phase):
        self._backup.CreateBackupInst.argtypes = [c_char_p, c_char_p, c_char_p, c_int, c_bool]
        self._backup.CreateBackupInst.restype = c_void_p
        meta_path = os.path.realpath(os.path.join(self._meta_path, 'latest'))
        instance = self._backup.CreateBackupInst(source.encode(), destination.encode(), meta_path.encode(), phase,
                                                 self._write_meta)
        self._configure_backup(instance)
        return instance

    def _configure_backup(self, instance):
        self._backup.ConfigureThreadPool.argtypes = [c_void_p, c_int, c_int]
        self._backup.ConfigureThreadPool.restype = c_int
        self._backup.ConfigureThreadPool(instance, self._thread_num, self._thread_num)
        self._backup.ConfigureMemory.argtypes = [c_void_p, c_int]
        self._backup.ConfigureMemory.restype = c_int
        self._backup.ConfigureMemory(instance, self._max_memory)

    def _enqueue(self, instance, backup_ctl_file):
        self._backup.Enqueue.argtypes = [c_void_p, c_char_p]
        self._backup.Enqueue.restype = c_int
        return self._backup.Enqueue(instance, backup_ctl_file.encode())

    def _start(self, instance):
        self._backup.Start.argtypes = [c_void_p]
        self._backup.Start.restype = c_int
        return self._backup.Start(instance)

    def _get_status(self, instance):
        self._backup.GetStatus.argtypes = [c_void_p]
        self._backup.GetStatus.restype = c_int
        return self._backup.GetStatus(instance)

    def _get_stats(self, instance, stats):
        self._backup.GetStats.argtypes = [c_void_p, POINTER(BackupStatistics)]
        self._backup.GetStats(instance, stats)

    def _destroy(self, instance):
        if instance:
            self._backup.DestroyBackupInst.argtypes = [c_void_p]
            self._backup.DestroyBackupInst(instance)
