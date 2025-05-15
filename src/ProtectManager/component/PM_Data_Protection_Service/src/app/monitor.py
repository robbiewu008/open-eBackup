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
import time
import subprocess
import threading
import os

from app.common import logger

log = logger.get_logger(__name__)


# 获取系统内存使用情况
def get_system_memory_info():
    try:
        with open('/proc/meminfo', 'r') as f:
            mem_info = f.readlines()
        memory_data = {}
        for line in mem_info:
            parts = line.split(":")
            if len(parts) == 2:
                key = parts[0].strip()
                value = parts[1].strip().split()[0]  # 获取数值部分，单位是 kB
                memory_data[key] = int(value)

        return memory_data
    except FileNotFoundError as e:
        log.error(f"Error reading /proc/meminfo: {e}")
        return {"error": f"Error reading /proc/meminfo: {str(e)}"}


# 获取当前进程的内存使用情况和CPU使用率
def get_process_info_with_top():
    pid = os.getpid()  # 获取当前进程的 PID
    try:
        # 使用 top 命令批处理模式获取当前进程的 CPU 和内存使用情况
        result = subprocess.run(
            ['/usr/bin/top', '-n', '1', '-b', '-p', str(pid)],
            capture_output=True, text=True
        )
        output = result.stdout.splitlines()

        # 查找包含 PID 的行并解析相应字段
        for line in output:
            if str(pid) in line:
                fields = line.split()
                cpu_usage = fields[8]  # CPU 使用率通常在第 9 列
                mem_usage = fields[9]  # MEM 使用率通常在第 10 列
                res_memory = int(fields[5]) / 1024  # RES 通常在第 6 列，转换为 MB
                shr_memory = int(fields[6]) / 1024  # SHR 通常在第 7 列，转换为 MB

                # 返回字典形式
                return {
                    "cpu_usage": cpu_usage,
                    "mem_usage": mem_usage,
                    "res_memory_mb": round(res_memory, 2),  # 转为 MB，保留两位小数
                    "shr_memory_mb": round(shr_memory, 2)  # 转为 MB，保留两位小数
                }
        return {"error": "Failed to retrieve information for PID {}".format(pid)}
    except Exception as e:
        return {"error": f"Error get process info with top: {e}"}


# 监控内存使用情况并打印
def memory_monitor(interval=120):
    while True:
        # 获取系统内存信息
        system_memory_info = get_system_memory_info()
        if "error" in system_memory_info:
            system_memory_log = "Unable to read system memory information."
        else:
            total_memory = system_memory_info.get('MemTotal', 0) / 1024  # 转为 MB
            free_memory = system_memory_info.get('MemFree', 0) / 1024
            available_memory = system_memory_info.get('MemAvailable', 0) / 1024
            used_memory = (total_memory - free_memory)  # 已使用内存

            system_memory_log = (f"System memory usage: Total: {total_memory:.2f} MB, "
                                 f"Free: {free_memory:.2f} MB, Available: {available_memory:.2f} MB, "
                                 f"Used: {used_memory:.2f} MB")

        # 获取进程内存和cpu信息
        process_memory_info = get_process_info_with_top()
        if "error" in process_memory_info:
            process_memory_log = "Unable to read process memory information."
        else:
            # 获取并处理内存和CPU信息
            process_cpu_info = process_memory_info.get('cpu_usage', None)
            process_resident_memory = process_memory_info.get('res_memory_mb', None)
            process_shared_memory = process_memory_info.get('shr_memory_mb', None)
            mem_usage = process_memory_info.get('mem_usage', None)

            process_memory_log = (f"Current process cpu usage: {process_cpu_info}%; memory usage: {mem_usage}%,"
                                  f"Resident: {process_resident_memory:.2f} MB, Shared: {process_shared_memory:.2f} MB")

        # 打印内存信息
        log.info(f"{system_memory_log}; {process_memory_log}")

        time.sleep(interval)


# 启动内存监控
def start_memory_monitor(interval=120):
    monitor_thread = threading.Thread(target=memory_monitor, args=(interval,), daemon=True)
    monitor_thread.start()