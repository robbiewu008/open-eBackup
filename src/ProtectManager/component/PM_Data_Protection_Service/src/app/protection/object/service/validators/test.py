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
import shutil


def find_target_folders(source_directory):
    """找目标"""
    targets = []
    for root, dirs, files in os.walk(source_directory):
        # 检查每个文件夹是否包含com/huawei/oceanprotect结构
        if 'com' in dirs and os.path.exists(os.path.join(root, 'com', 'huawei', 'oceanprotect')):
            # 获取com/huawei/oceanprotect文件夹的路径
            targets.append(os.path.join(root, 'com', 'huawei', 'oceanprotect'))
    return targets

def move_dir(targets):
    """移动目标, 删除空文件"""
    new_dirs = []
    for target in targets:
        root = target.split('\\com')[0]
        root = "\\\\?\\" + root
        # 移动文件
        sources = os.listdir(target)
        new_dir = os.path.join(root, "openbackup")
        if not os.path.exists(new_dir):
            os.mkdir(new_dir)
        for source in sources:
            source_path = os.path.join(target, source)
            shutil.move(source_path, new_dir)
        new_dirs.append(new_dir)
        #删除目录
        shutil.rmtree(os.path.join(root, "com"))
    return new_dirs

def modify_files(new_dirs, old_string, new_string):
    for dir in new_dirs:
        # 遍历目录
        for root, dirs, files in os.walk(dir):
            # 遍历文件
            for f in files:
                # 文件路径
                file_path = os.path.join(root, f)
                print(file_path)
                # 打开文件并读取内容
                with open(file_path, 'r', encoding='UTF-8') as file:
                    lines = file.readlines()
                new_lines = []
                for i, line in enumerate(lines):
                    if "public" in line:
                        new_lines.extend(lines[i:])
                        break
                    if old_string in line:
                        # 替换字符串
                        line = line.replace(old_string, new_string)
                    new_lines.append(line)

                # 写入修改后的内容
                with open(file_path, 'w', encoding='UTF-8') as f:
                    f.writelines(new_lines)



if __name__ == "__main__":
    # 找出所有符合条件的目录
    source_directory = input("请输入目录绝对路径:\t")
    targets = find_target_folders(source_directory)
    print(f"将会对以下目录进行修改{targets}")

    # 将com/huawei/oceanprotect下的所有文件拷贝到openbackup文件夹下
    new_dirs = move_dir(targets)
    print("文件已迁移到指定目录")

    # 修改openbackup文件夹下的所有文件中的字符串com.huawei.oceanprotect为openbackup
    modify_files(new_dirs, 'com.huawei.oceanprotect', 'openbackup')
    print("******************")
    print("文件修改成功")
    print("******************")