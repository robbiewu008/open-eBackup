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
import configparser
import platform

from ctypes import cdll, Structure, c_char_p, c_void_p, c_uint64, c_bool
from pydantic import BaseModel, Field
from common.env_common import get_install_head_path, adaptation_win_path

if platform.system().lower() == "windows":
    _SCANNER_PATH = f"{adaptation_win_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/scantmp"
    _LOG_LEVEL_PATH = f"{adaptation_win_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/confhcpconf.ini"
else:
    _SCANNER_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/scantmp"
    _LOG_LEVEL_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/conf/hcpconf.ini"


def get_log_level():
    log_dict = {"DEBUG": 0, "INFO": 1, "WARNING": 2, "ERROR": 3, "CRITICAL": 4}
    if not os.path.isfile(_LOG_LEVEL_PATH):
        return log_dict.get("INFO")
    read_ini = configparser.ConfigParser()
    read_ini.read(_LOG_LEVEL_PATH)
    value = read_ini.get("General", "LogLevel")
    if int(value) < len(log_dict):
        return int(value)
    return log_dict.get("INFO")


class ScanStats(BaseModel):
    open_dir_request_cnt: int = Field(None, description="打开目录请求次数", alias="openDirRequestCnt")
    open_dir_time: int = Field(None, description="打开目录时间", alias="openDirTime")
    total_dirs: int = Field(None, description="总目录数", alias="totalDirs")
    total_files: int = Field(None, description="总文件数", alias="totalFiles")
    total_size: int = Field(None, description="总大小", alias="totalSize")
    total_failed_dirs: int = Field(None, description="失败目录数", alias="totalFailedDirs")
    total_failed_files: int = Field(None, description="失败文件数", alias="totalFailedFiles")
    total_control_files: int = Field(None, description="控制文件数量", alias="totalControlFiles")
    total_size_to_backup: int = Field(None, description="需要备份对象数", alias="totalSizeToBackup")
    total_dirs_to_backup: int = Field(None, description="需要备份目录数", alias="totalDirsToBackup")
    total_files_to_backup: int = Field(None, description="需要备份文件数", alias="totalFilesToBackup")


class ScanConf(Structure):
    _fields_ = [
        ("jobId", c_char_p),
        ("metaPath", c_char_p),
        ("metaPathForCtrlFiles", c_char_p),
        ("enableProduce", c_bool),
        ("writeQueueSize", c_uint64)
    ]


class ScanStatS(Structure):
    _fields_ = [
        ("openDirRequestCnt", c_uint64),
        ("openDirTime", c_uint64),
        ("totalDirs", c_uint64),
        ("totalFiles", c_uint64),
        ("totalSize", c_uint64),
        ("totalFailedDirs", c_uint64),
        ("totalFailedFiles", c_uint64),
        ("totalControlFiles", c_uint64),
        ("totalSizeToBackup", c_uint64),
        ("totalDirsToBackup", c_uint64),
        ("totalFilesToBackup", c_uint64)
    ]


class Scanner(object):
    def __init__(self):
        if platform.system().lower() == "windows":
            lib_path = f"{adaptation_win_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin"
            dll_path = os.path.join(lib_path, "FS_Scanner.dll").replace('\\', '/')
            self._scanner = cdll.LoadLibrary(dll_path)
        elif platform.system().lower() == "aix":
            lib_path = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/lib"
            self._scanner = cdll.LoadLibrary(os.path.join(lib_path, "libScanner.a(libScanner.so)"))
        else:
            lib_path = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/lib"
            self._scanner = cdll.LoadLibrary(os.path.join(lib_path, "libScanner.so"))
        self._scanner.InitLog.argtypes = [c_char_p]
        if platform.system().lower() == "windows":
            log_path = f"{adaptation_win_path()}/DataBackup/ProtectClient/ProtectClient-E/log/Plugins/GeneralDBPlugin/"
        else:
            log_path = os.getenv("GENERALDB_LOG_PATH")
        log_level = get_log_level()
        self._scanner.InitLog(log_path.encode(), log_level)
        self._instance = None

    def __del__(self):
        self.destroy()

    def create(self, job_id, meta_path, meta_path_for_ctrl_files, enable_produce=True, write_queue_size=1000):
        scan_conf = ScanConf(job_id.encode(), meta_path.encode(), meta_path_for_ctrl_files.encode(),
                             enable_produce, write_queue_size)
        self._scanner.CreateScannerInst.argtypes = [ScanConf]
        self._scanner.CreateScannerInst.restype = c_void_p
        self._instance = self._scanner.CreateScannerInst(scan_conf)

    def start(self, dir_path):
        if platform.system().lower() == "windows":
            self._scanner.StartScannerWithoutSplitCompoundPrefix.argtypes = [c_void_p, c_char_p]
            self._scanner.StartScannerWithoutSplitCompoundPrefix.restype = c_bool
            return self._scanner.StartScannerWithoutSplitCompoundPrefix(self._instance, dir_path.encode())
        else:
            self._scanner.StartScanner.argtypes = [c_void_p, c_char_p]
            self._scanner.StartScanner.restype = c_bool
            return self._scanner.StartScanner(self._instance, dir_path.encode())

    def start_scan_files(self, files):
        self._scanner.StartScannerBatch.argtypes = [c_void_p, c_char_p]
        self._scanner.StartScannerBatch.restype = c_bool
        files_str = str()
        for key in files:
            files_str += key
            if key != files[-1]:
                files_str += ";"
        return self._scanner.StartScannerBatch(self._instance, files_str.encode())

    def monitor(self, job_id):
        self._scanner.MonitorScanner.argtypes = [c_void_p, c_char_p]
        self._scanner.MonitorScanner.restype = c_bool
        return self._scanner.MonitorScanner(self._instance, job_id.encode())

    def get_stats(self):
        self._scanner.GetStatistics.argtypes = [c_void_p]
        self._scanner.GetStatistics.restype = ScanStatS
        scan_stats = self._scanner.GetStatistics(self._instance)
        stats = ScanStats(openDirRequestCnt=scan_stats.openDirRequestCnt,
                          openDirTime=scan_stats.openDirTime,
                          totalDirs=scan_stats.totalDirs,
                          totalFiles=scan_stats.totalFiles,
                          totalSize=scan_stats.totalSize,
                          totalFailedDirs=scan_stats.totalFailedDirs,
                          totalFailedFiles=scan_stats.totalFailedFiles,
                          totalControlFiles=scan_stats.totalControlFiles,
                          totalSizeToBackup=scan_stats.totalSizeToBackup,
                          totalDirsToBackup=scan_stats.totalDirsToBackup,
                          totalFilesToBackup=scan_stats.totalFilesToBackup)
        return stats.dict(by_alias=True)

    def destroy(self):
        self._scanner.DestroyScannerInst.argtypes = [c_void_p]
        self._scanner.DestroyScannerInst(self._instance)


def scan_dir_size(job_id, dirs):
    """
    return unit is kb
    """
    scanner = Scanner()
    meta_path = os.path.realpath(os.path.join(_SCANNER_PATH, job_id, "meta"))
    meta_ctl_path = os.path.realpath(os.path.join(_SCANNER_PATH, job_id, "meta_ctl"))

    if not os.path.exists(_SCANNER_PATH):
        os.makedirs(_SCANNER_PATH)
    if not os.path.exists(meta_path):
        os.makedirs(meta_path)
    if not os.path.exists(meta_ctl_path):
        os.makedirs(meta_ctl_path)
    scanner.create(job_id, meta_path, meta_ctl_path, False)
    if not scanner.start(dirs):
        return False, 0
    if not scanner.monitor(job_id):
        return False, 0
    stats = scanner.get_stats()
    if stats.get("totalSize", 0) != 0:
        size = int(stats.get("totalSize", 0) / 1024)
    else:
        size = 0
    return True, size


def scan_dir(job_id, meta_ctl_path, directory):
    scanner = Scanner()
    meta_path = os.path.realpath(os.path.join(_SCANNER_PATH, job_id, "meta"))
    if not os.path.exists(_SCANNER_PATH):
        os.makedirs(_SCANNER_PATH)
    if not os.path.exists(meta_path):
        os.makedirs(meta_path)
    if not os.path.exists(meta_ctl_path):
        os.makedirs(meta_ctl_path)
    if platform.system().lower() != "windows":
        directory = os.path.join(directory, "")
    else:
        directory = directory.replace('/', '\\')
    scanner.create(job_id, meta_path, meta_ctl_path)
    if not scanner.start(directory):
        return False
    if not scanner.monitor(job_id):
        return False
    return True