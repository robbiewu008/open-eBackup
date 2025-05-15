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
import os
import re
import shutil
import compileall
from pathlib import Path

versions = None
PYCACHE = "__pycache__"
# 源代码路径，相较于工程目录
SOURCE_CODE_DIR = "src/common"
# pyc代码路径，相较于工程目录
TARGET_CODE_DIR = "src/common_pyc"
# main pyc文件第一次编译后的路径，相较于工程目录
MAIN_DIR = TARGET_CODE_DIR + os.sep + PYCACHE


def get_root_path():
    current = os.getcwd()
    return Path(current).parent


def move_source_code_to_build_dir(build_dir):
    root_path = get_root_path()
    source_path = Path(root_path, SOURCE_CODE_DIR)
    tmp_build_path = Path(root_path, build_dir)
    if tmp_build_path.exists():
        shutil.rmtree(tmp_build_path)
    shutil.copytree(source_path, tmp_build_path)
    return tmp_build_path


def get_versions_from_main():
    root_path = get_root_path()
    main_dir_path = Path(root_path, MAIN_DIR)

    main_dir_files = os.listdir(main_dir_path)
    for file in main_dir_files:
        aaa = re.match(r'.*(\..*?)\.py', file)
        if aaa:
            return aaa.group(1)
    return None


def resolve_build_dir(build):
    for root, _, files in os.walk(build):  # 开始遍历文件
        # root 表示当前正在访问的文件夹路径
        # dirs 表示该文件夹下的子目录名list
        # files 表示该文件夹下的文件list
        # 遍历文件
        for f in files:
            src = os.path.join(root, f)
            if f.endswith(".py"):
                os.remove(src)
            elif f.endswith(".pyc"):
                upper_dir = root.replace(PYCACHE, "")
                dst = os.path.join(upper_dir, f.replace(versions, ""))
                shutil.copy(src, dst)
    for root, _, _ in os.walk(build):  # 移除缓存文件夹
        if root.endswith(PYCACHE):
            shutil.rmtree(root)


if __name__ == '__main__':
    # 移动原代码文件到构建目录
    build_path = move_source_code_to_build_dir(TARGET_CODE_DIR)
    # 构建目录的文件编译成pyc
    compileall.compile_dir(build_path.as_posix())
    versions = get_versions_from_main()
    if not versions:
        raise Exception("can not get version")
    # 调整构建目录的pyc文件结构使之与原有目录结构一致，并删除构建目录的py文件
    resolve_build_dir(build_path)
