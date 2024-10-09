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

import glob
import os

from mysql import log
from common.file_common import exec_lchown_dir_recursively, get_user_info
from common.util.exec_utils import su_exec_rm_cmd, exec_mv_cmd
from mysql.src.common.constant import RestorePath, MySQLStrConstant
from mysql.src.utils.mysql_utils import MysqlUtils


class MysqlUndoService:
    @staticmethod
    def get_undo_dir(my_cnf_path):
        """
        获取undo文件的目录。如果从配置文件中无法读取undo目录，为undo为默认的数据目录
        :return:
        """
        innodb_undo_directory = MysqlUtils.get_innodb_undo_directory_by_config_file(my_cnf_path)
        if innodb_undo_directory == "" or innodb_undo_directory is None:
            innodb_undo_directory = MysqlUtils.get_data_dir_by_config_file(my_cnf_path)
        return innodb_undo_directory

    @staticmethod
    def backup_undo_files(job_id: str, my_cnf_path):
        """
        将当前环境的undo文件，在当前环境的undo目录下备份一份，以.old结尾。如果没有undo文件，则忽略
        :return:
        """
        innodb_undo_directory = MysqlUndoService.get_undo_dir(my_cnf_path)
        if not os.path.exists(innodb_undo_directory):
            log.info(f"No undo files. job id: {job_id}")
            return
        undo_file_list = glob.glob(os.path.join(innodb_undo_directory, MySQLStrConstant.UNDO_PREFIX + '*'))
        for undo_file_path in undo_file_list:
            exec_mv_cmd(undo_file_path, f"{undo_file_path}{RestorePath.DOTOLD}", check_white_black_list_flag=False)
            log.info(f"Backup undo file: {undo_file_path} success. jod id: {job_id}")

    @staticmethod
    def delete_undo_files(job_id: str, my_cnf_path):
        innodb_undo_directory = MysqlUndoService.get_undo_dir(my_cnf_path)
        if not os.path.exists(innodb_undo_directory):
            log.info(f"No undo files. job id: {job_id}")
            return
        undo_file_list = glob.glob(os.path.join(innodb_undo_directory, MySQLStrConstant.UNDO_PREFIX + '*'))
        # 循环遍历文件列表并删除每个文件
        for undo_file_path in undo_file_list:
            # .old是备份的原来的undo文件，需要在恢复后起服务前恢复回去的，不能删除
            if undo_file_path.endswith(RestorePath.DOTOLD):
                continue
            if not su_exec_rm_cmd(undo_file_path, check_white_black_list_flag=False):
                raise Exception(f"The path[{undo_file_path}] remove failed. jod id: {job_id}")
            log.info(f"Delete undo file: {undo_file_path} success. jod id: {job_id}")

    @staticmethod
    def restore_undo_files(job_id: str, my_cnf_path):
        """
        在备份工具的恢复命令执行后，在起mysql服务前，需要用备份的undo文件，恢复回去。
        场景：在数据库恢复时，会将原来的undo恢复到新环境，所以我们需要先备份以前的，在恢复命令执行后，在恢复回去。
        :param job_id:
        :param my_cnf_path:
        :return:
        """
        innodb_undo_directory = MysqlUndoService.get_undo_dir(my_cnf_path)
        if not os.path.exists(innodb_undo_directory):
            log.info(f"No undo files. job id: {job_id}")
            return
        undo_file_list = glob.glob(os.path.join(innodb_undo_directory, MySQLStrConstant.UNDO_PREFIX + '*'))
        for undo_file_path in undo_file_list:
            # 先删除非.old结尾的
            if undo_file_path.endswith(RestorePath.DOTOLD):
                continue
            if not su_exec_rm_cmd(undo_file_path, check_white_black_list_flag=False):
                raise Exception(f"The path[{undo_file_path}] remove failed. jod id: {job_id}")
        for undo_file_path in undo_file_list:
            # 恢复undo文件回去
            new_name = undo_file_path.replace(RestorePath.DOTOLD, "")
            exec_mv_cmd(f"{undo_file_path}", new_name, check_white_black_list_flag=False)
            log.info(f"Restore undo file: {undo_file_path} success. jod id: {job_id}")

    @staticmethod
    def change_undo_files_permission(job_id: str, my_cnf_path):
        """
        修改undo文件夹下的undo文件权限，也可能不存在
        :return:
        """
        innodb_undo_directory = MysqlUndoService.get_undo_dir(my_cnf_path)
        if not os.path.exists(innodb_undo_directory):
            log.info(f"No undo files. job id: {job_id}")
            return True
        file_list = glob.glob(os.path.join(innodb_undo_directory, MySQLStrConstant.UNDO_PREFIX + '*'))
        for file_path in file_list:
            if MysqlUndoService.change_permission(file_path, my_cnf_path):
                log.info(f"Undo file path: {file_path} chown success. jod id: {job_id}")
            else:
                return False
        return True

    @staticmethod
    def change_permission(file_path: str, my_cnf_path: str):
        user_name = MysqlUtils.get_data_path_user_name(my_cnf_path)
        user_group, user_info = get_user_info(user_name)
        ret = exec_lchown_dir_recursively(file_path, user_name, user_group)
        if not ret:
            log.error(f"Exec chown mysql failed")
            return False
        return True

    @staticmethod
    def clean_undo_tmp_dir(job_id: str, cache_path: str):
        """
        删除可能存在的undo临时文件夹
        :param job_id: 任务id
        :param cache_path: 缓存仓地址
        :return:
        """
        innodb_undo_directory_tmp = cache_path + "/" + job_id
        if not os.path.exists(innodb_undo_directory_tmp):
            log.info(f"No undo tmp dir. job id: {job_id}")
            return
        if not su_exec_rm_cmd(innodb_undo_directory_tmp):
            log.error(f"Rm file failed. path:[{innodb_undo_directory_tmp}]. jobId:{job_id}")
        log.info(f"Clean undo tmp dir: {innodb_undo_directory_tmp} success. jod id: {job_id}")
