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
import sys
import argparse
from concurrent.futures import ThreadPoolExecutor
from multiprocessing import cpu_count
from functools import partial
import shutil
import platform
# 将根目录添加到 Python 路径
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), './applications')))
from common.util.scanner_utils import scan_dir
exclude_files = ["DataRecordFile.txt", "MetaRecordFile.txt", "DataRecordDir.txt", "MetaRecordDir.txt"]


def parse_dir(dir_path):
    length = len(dir_path)
    if platform.system().lower() == "windows" and length >= 2:
        dir_path = dir_path[2:]
    dir_path = dir_path.lstrip('/')
    return dir_path


def process_file(file_path, scan_path):
    """
    处理单个文件，提取目录和文件信息。
    返回一个元组 (directories, files)。
    """
    directories = []
    files = []
    current_dir = ""
    file_path = file_path.replace("\\", "/")
    with open(file_path, "r") as file:
        for line in file:
            parts = line.strip().split(",")
            if len(parts) < 4:
                continue
            marker = parts[0].strip()
            path = parts[3].strip()
            if marker == "d":
                current_dir = parse_dir(path)
                current_dir = scan_path.rstrip('/') + '/' + current_dir
                directories.append(current_dir)
            elif marker == "f" and path not in exclude_files:
                path = os.path.join(current_dir, path).replace("\\", "/")
                files.append(path)
    return directories, files


def write_results_to_files(directories, files, dir_output_path, file_output_path):
    """
    将目录和文件信息分别写入到指定的文件中。
    """
    with open(dir_output_path, "a") as dir_file, open(file_output_path, "a") as file_file:
        for directory in directories:
            dir_file.write(directory + "\n")
        for file in files:
            file_file.write(file + "\n")


def generate_records(repo_type, crl_path, save_path, truncate_scan_path):
    if not os.path.exists(save_path):
        os.makedirs(save_path)
    process_file_partial = partial(process_file, scan_path=truncate_scan_path)
    file_paths = [os.path.join(crl_path, f) for f in os.listdir(crl_path) if f.startswith("control")]
    with ThreadPoolExecutor(max_workers=cpu_count()) as executor:
        results = list(executor.map(process_file_partial, file_paths))

    # 收集所有结果
    dir_output_path = save_path + '/' + repo_type + "RecordDir.txt"
    file_output_path = save_path + '/' + repo_type + "RecordFile.txt"
    ctrl_string = os.path.basename(crl_path)
    for directories, files in results:
        directories = [direct for direct in directories if not direct.endswith(ctrl_string)]
        write_results_to_files(directories, files, dir_output_path, file_output_path)


if __name__ == '__main__':
    # 校验
    if len(sys.argv) < 3:
        print("Number of argv wrong. ")
        sys.exit(1)
    parser = argparse.ArgumentParser(description="Process paths and a list.")
    func_type = sys.argv[1]
    if func_type == "scanRepo":
        # 添加任务操作函数
        parser.add_argument("func_type", type=str, help="Func type")
        # 添加任务id参数
        parser.add_argument("job_id", type=str, help="Job id")
        # 添加路径参数
        parser.add_argument("crl_path", type=str, help="ctrl path")
        # 添加扫描路径
        parser.add_argument("scan_path", type=str, help="scan path")
        # 解析命令行参数
        args = parser.parse_args()
        print(f"crl_path: {args.crl_path}, scan_dir: {args.scan_path}")
        scan_dir(args.job_id, args.crl_path, args.scan_path)
    elif func_type == "generateRecords":
        # 添加任务操作函数
        parser.add_argument("func_type", type=str, help="Func type")
        # 添加任务id参数
        parser.add_argument("job_id", type=str, help="Job id")
        # 添加仓库类型
        parser.add_argument("repo_type", type=str, help="Type")
        # 添加路径参数
        parser.add_argument("crl_path", type=str, help="ctrl path")
        # 添加扫描路径
        parser.add_argument("save_path", type=str, help="save path")
        # 文件保存路径
        parser.add_argument("truncate_path", type=str, help="prefix")
        args = parser.parse_args()
        print(f"crl_path: {args.crl_path}, save_path: {args.save_path}, truncate_path: {args.truncate_path}")
        generate_records(args.repo_type, args.crl_path, args.save_path, args.truncate_path)
        # ctrl目录解析完后删除
        if os.path.isdir(args.crl_path):
            shutil.rmtree(args.crl_path)
    sys.exit(0)