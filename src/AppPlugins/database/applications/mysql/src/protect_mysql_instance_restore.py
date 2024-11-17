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
import uuid

from common.common import exter_attack, check_command_injection_exclude_quote, is_clone_file_system
from common.const import SubJobStatusEnum
from common.file_common import get_user_info, exec_lchown_dir_recursively
from common.util.check_utils import check_repo_path
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import su_exec_rm_cmd, exec_mkdir_cmd, exec_cat_cmd, exec_mv_cmd, exec_cp_cmd
from mysql import log
from mysql.src.common.constant import MySQLStrConstant, RestorePath, MySQLRestoreStep, MysqlBackupToolName
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.execute_cmd import check_delete_mysql_bin_log_file, \
    is_certificate_file
from mysql.src.protect_mysql_base import RestoreType
from mysql.src.protect_mysql_restore_base import MysqlRestoreBase
from mysql.src.protect_mysql_restore_common import MysqlRestoreCommon, G_NEED_PERPARE_MYSQL_TYPE
from mysql.src.protect_mysql_undo_service import MysqlUndoService
from mysql.src.utils.mysql_job_param_util import MysqlJobParamUtil
from mysql.src.utils.mysql_service_info_utils import MysqlServiceInfoUtil
from mysql.src.utils.mysql_utils import MysqlUtils


class MysqlInstanceRestore(MysqlRestoreBase):
    def __init__(self, p_id, job_id, sub_job_id, json_param):
        super().__init__(p_id, job_id, sub_job_id, json_param)
        self._restore_step = 0
        self._error_code = 0

    def operate_non_common_node(self, restore_type, mysql_data_dir):
        if restore_type != RestoreType.EAPP_INSTANCE \
                and self._restore_step >= MySQLRestoreStep.OPERATE_NON_COMMON_NODE:
            log.debug(f"Step {MySQLRestoreStep.OPERATE_NON_COMMON_NODE} done. pid:{self._p_id} jobId:{self._job_id}")
            return True
        if restore_type == RestoreType.PXC_INSTANCE_COMMON_NODE:
            log.debug(f"Get restore_type: {restore_type} equal PXC INSTANCE COMMON NODE")
            self.write_restore_step_info(MySQLRestoreStep.OPERATE_NON_COMMON_NODE)
            return True
        if not os.path.exists(mysql_data_dir):
            self.write_restore_step_info(MySQLRestoreStep.OPERATE_NON_COMMON_NODE)
            return True
        log.debug(f"mysql_data_dir exists.")
        if os.path.exists(mysql_data_dir + RestorePath.DOTOLD):
            if not su_exec_rm_cmd(mysql_data_dir + RestorePath.DOTOLD, check_white_black_list_flag=False):
                log.error(f"Rm file failed. path:[{mysql_data_dir + RestorePath.DOTOLD}]. "
                          f"pid:{self._p_id} jobId:{self._job_id}")
                return False
        user_name = MysqlUtils.get_data_path_user_name(self.my_cnf_path)
        user_group, user_info = get_user_info(user_name)
        log.info(f"user_name:{user_name},user_group:{user_group}")
        try:
            ret = exec_mv_cmd(mysql_data_dir, f"{mysql_data_dir}{RestorePath.DOTOLD}",
                              check_white_black_list_flag=False)
            if not ret:
                exec_mkdir_cmd(f"{mysql_data_dir}{RestorePath.DOTOLD}", is_check_white_list=False)
                exec_lchown_dir_recursively(f"{mysql_data_dir}{RestorePath.DOTOLD}", user_name, user_group)
                exec_lchown_dir_recursively(mysql_data_dir, user_name, user_group)
        except Exception as exception_str:
            log.warning(f"Rename data path failed.{exception_str}")
            self.clean_dir(mysql_data_dir)
        self.write_restore_step_info(MySQLRestoreStep.OPERATE_NON_COMMON_NODE)
        return True

    def call_xtrabackup(self, restore_type, copy_path, mysql_data_dir):
        """
        在mysql 8.x中，如果实例恢复时，如果指定了undo相关参数，虽然恢复命令不会报错，但是mysql在启动服务时，
        会先从指定的这个目录undo文件，然后解析undo文件，并重新在新环境的undo目录下生成，且权限自动就是mysql了
        所以这个目录，我们不能指定为新环境的undo目录，否则mysql启动服务会失败
        然后再在新环境的undo目录生成新的undo文件，这种新的undo文件权限就自动改好了
        在mysql 5.x和mariadb 10.x中，如果指定了undo相关参数，则会自动恢复undo文件到对应的目录
        mysql不会生成新的undo文件，且文件的权限也需要修改
        ps：其实也可以不指定undo参数，备份工具会自动将undo文件恢复到undo目录的，权限依然需要手动修改
        :param restore_type: restore_type
        :param copy_path: copy_path
        :param mysql_data_dir: mysql_data_dir
        :return: True or False
        """
        if RestoreType.EAPP_INSTANCE != restore_type \
                and self._restore_step >= MySQLRestoreStep.XTRABACKUP_RESTORE:
            log.debug(f"Step {MySQLRestoreStep.XTRABACKUP_RESTORE} has done. pid:{self._p_id} jobId:{self._job_id}")
            return True
        if restore_type == RestoreType.PXC_INSTANCE_COMMON_NODE:
            self.write_restore_step_info(MySQLRestoreStep.XTRABACKUP_RESTORE)
            return True
        # 恢复前删除目标实例的日志目录
        if not self.delete_target_log_bin_path(copy_path, mysql_data_dir):
            log.error("Delete target log bin file failed.")
            return False
        channel_number = self.get_channel_number()
        tool_name = self.get_backup_tool_name()
        if tool_name != MysqlBackupToolName.MARIADBBACKUP and restore_type in G_NEED_PERPARE_MYSQL_TYPE:
            log.info(f"Start prepare. pid:{self._p_id} jobId:{self._job_id}")
            # mysql 5.7 8.0需要再恢复前执行prepare做一次未提交事务的回滚
            prepare_param = cmd_format("--prepare {} --parallel={} --target-dir={}",
                                       self._restore_comm.get_force_recover_str(), channel_number, copy_path)
            ret, output = self.exec_xtrabackup_cmd(prepare_param, tool_name)
            if not ret:
                log.error(f"Exec prepare cmd failed. output:{output} pid:{self._p_id} jobId:{self._job_id}")
                return False
        restore_way = "--copy-back" if is_clone_file_system(self.get_json_param()) else "--move-back"
        restore_cmd_param = cmd_format(" {} --parallel={} --datadir={} {} --target-dir={}",
                                       self._restore_comm.get_force_recover_str(), channel_number,
                                       mysql_data_dir, restore_way, copy_path)
        restore_cmd_param_undo = self.init_restore_cmd_param_undo(restore_cmd_param)
        ret, output = self.exec_xtrabackup_cmd(restore_cmd_param_undo, tool_name)
        if not ret:
            log.error(f"Exec restore cmd failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        log.info(f"Exec restore cmd success. pid:{self._p_id} jobId:{self._job_id}")
        self.write_restore_step_info(MySQLRestoreStep.XTRABACKUP_RESTORE)
        return True

    def init_restore_cmd_param_undo(self, restore_cmd_param):
        undo_size = MysqlServiceInfoUtil.get_undo_size_in_point_dir(self.get_full_copy_path())
        log.info(f"In backup copy undo size: {undo_size}, pid:{self._p_id} jobId:{self._job_id}")
        restore_cmd_param_undo = restore_cmd_param
        if undo_size != 0:
            version = MysqlJobParamUtil.get_mysql_version(self.get_json_param(), self._job_id, self._p_id)
            if MySQLStrConstant.MARIADB in version or version.startswith("5."):
                innodb_undo_directory = MysqlUndoService.get_undo_dir(self.my_cnf_path)
                restore_cmd_param_undo = restore_cmd_param + cmd_format(
                    " --innodb-undo-tablespaces={} --innodb-undo-directory={}", undo_size, innodb_undo_directory)
            else:
                innodb_undo_directory_tmp = f"{self.get_full_cache_path()}/{str(uuid.uuid4())}/"
                self.create_innodb_undo_directory_tmp(innodb_undo_directory_tmp)
                restore_cmd_param_undo = restore_cmd_param + cmd_format(
                    " --innodb-undo-tablespaces={} --innodb-undo-directory={}", undo_size, innodb_undo_directory_tmp)
            # 不管哪个版本的mysql实例恢复，恢复前要删除新环境的undo文件夹下的undo文件，否则要么恢复命令失败，要么启动服务失败
            MysqlUndoService.delete_undo_files(self._job_id, self.my_cnf_path)
        return restore_cmd_param_undo

    def create_innodb_undo_directory_tmp(self, innodb_undo_directory_tmp):
        if not os.path.exists(innodb_undo_directory_tmp) and exec_mkdir_cmd(innodb_undo_directory_tmp):
            log.error(f"Mkdir failed. pid:{self._p_id} jobId:{self._job_id}")

    def rename_files_before_restore(self, restore_type, mysql_data_dir):
        if self._restore_step >= MySQLRestoreStep.RENAME_BEFORE_RESTORE:
            log.debug(f"Step {MySQLRestoreStep.RENAME_BEFORE_RESTORE} has done. pid:{self._p_id} jobId:{self._job_id}")
            return True
        if restore_type != RestoreType.AP_INSTANCE_CLUSTER_SLAVE_NODE:
            log.debug(f"Get restore_type: {restore_type} not equal AP INSTANCE CLUSTER SLAVE NODE")
            self.write_restore_step_info(MySQLRestoreStep.RENAME_BEFORE_RESTORE)
            return True
        self.del_dot_old_dir_or_file(mysql_data_dir)
        self.write_restore_step_info(MySQLRestoreStep.RENAME_BEFORE_RESTORE)
        return True

    def delete_target_log_bin_path(self, copy_path, mysql_data_dir):
        """
        删除目标日志文件夹下的文件
        @param copy_path: 副本目录
        @param mysql_data_dir: data目录
        @return:True 删除成功 False 删除失败
        """
        mysql_binlog_dir = self.get_restore_binlog_dir()
        if mysql_data_dir in mysql_binlog_dir:
            log.info("The mysql log bin file is in mysql data dir.")
            return True
        # 拿到副本中将要移除的文件列表
        remove_bin_log_list = self.get_copy_bin_log_file_list(copy_path)
        if not remove_bin_log_list:
            log.info("There are no file in index or it is cluster restore.")
            return True
        # 删除目标实例的bin log
        for file in remove_bin_log_list:
            target_log_bin_file = os.path.join(mysql_binlog_dir, file)
            if (check_delete_mysql_bin_log_file(target_log_bin_file) and
                    not su_exec_rm_cmd(target_log_bin_file, check_white_black_list_flag=False)):
                log.error(f"Rm file failed. path:[{target_log_bin_file}]. pid:{self._p_id} jobId:{self._job_id}")
                return False
        log.warning(f"Delete log bin file list {remove_bin_log_list} success.")
        return True

    def get_copy_bin_log_file_list(self, copy_path):
        remove_bin_log_list = []
        for item in os.listdir(copy_path):
            new_path = os.path.join(copy_path, item)
            # 如果副本中是文件夹则直接跳过
            if os.path.isdir(new_path):
                continue
            # 否则找到下面的以index结尾的文件，并读取副本中的存在的index文件
            if not new_path.endswith(".index"):
                continue
            if not os.path.isfile(new_path) and check_command_injection_exclude_quote(new_path):
                log.error(f"Check index file failed.")
                return []
            if os.path.islink(new_path):
                log.error(f"Index file is link.")
                return []

            ret, out_put = exec_cat_cmd(new_path)
            if not ret:
                log.error(f"Read index file failed. ret:{ret} pid:{self._p_id} jobId:{self._job_id}")
                return []
            for index_item in out_put.split("\n"):
                if not index_item.startswith("/"):
                    continue
                remove_bin_log_list.append(index_item[index_item.rfind("/") + 1:])
            remove_bin_log_list.append(item)
        return remove_bin_log_list

    def operate_file_power_after_restore(self, mysql_data_dir):
        if not MysqlUndoService.change_undo_files_permission(self._job_id, self.my_cnf_path):
            return False
        log.info(f"start change permission,{mysql_data_dir}")
        mysql_log_bin_dir = self.get_restore_binlog_dir()
        log.info(f"mysql_log_bin_dir:{mysql_log_bin_dir}")
        if not self.operate_chown(mysql_log_bin_dir) or \
                not self.operate_chown(mysql_data_dir):
            return False
        if not self._restore_comm.chown_ibd_files(self.my_cnf_path):
            return False
        if not MysqlRestoreCommon.operate_chcon_contest(mysql_data_dir) or \
                not MysqlRestoreCommon.operate_chcon_contest(mysql_log_bin_dir):
            return False
        if not self._restore_comm.operate_chcon_contest_ibd():
            return False
        return True

    def prepare_restore(self, restore_type, mysql_data_dir, copy_path):
        if is_clone_file_system(self.get_json_param()) and not check_repo_path(copy_path):
            log.error("Check instance restore copy_path invalid.")
            return False
        if not self.validate_innodb_undo_size():
            return False
        if not self._restore_comm.deal_ibd_files(self._restore_comm.get_isl_file_list_by_copy(copy_path),
                                                 self.my_cnf_path):
            return False
        if not self.rename_files_before_restore(restore_type, mysql_data_dir):
            return False
        if not self.operate_non_common_node(restore_type, mysql_data_dir):
            return False
        return True

    def validate_innodb_undo_size(self):
        version = MysqlJobParamUtil.get_mysql_version(self.get_json_param(), self._job_id, self._p_id)
        log.info(f"MySQL/MariaDB version: {version}")
        if MysqlUtils.innodb_undo_tablespaces_deprecated(version):
            return True

        undo_size = MysqlServiceInfoUtil.get_undo_size_in_point_dir(self.get_full_copy_path())
        log.info(f"copy undo size: {undo_size}, pid:{self._p_id} jobId:{self._job_id}")
        cnf_undo_size = MysqlUtils.get_innodb_undo_tablespace_by_config_file(self.my_cnf_path)
        log.info(f"cnf undo size: {cnf_undo_size}, pid:{self._p_id} jobId:{self._job_id}")
        if str(undo_size) != str(cnf_undo_size):
            log.error("copy undo size not equals to cnf undo size")
            self._error_code = MySQLErrorCode.DIFF_INNODB_UNDO_TABLESPACES
            self.set_log_detail_params([str(cnf_undo_size), str(undo_size)])
            return False
        return True

    @exter_attack
    def exec_sub_job(self, restore_type, log_restore):
        if not self.check_mysql_cluster_is_stop():
            return False
        log.info(f"Instance restore begin. pid:{self._p_id} jobId:{self._job_id}")
        self._restore_step = self.read_restore_step_info()
        ret, copy_path = self.get_restore_copy_path()
        if not ret:
            log.error(f"Exec set restore all param failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        if not is_clone_file_system(self.get_json_param()):
            data_tmp_dir = os.path.join("/tmp", self._job_id)
            exec_cp_cmd(copy_path, data_tmp_dir, is_check_white_list=False)
            copy_path = data_tmp_dir
        progress_type = self._sub_job_id if self._sub_job_id else self._job_id
        self.write_progress_file(SubJobStatusEnum.RUNNING.value, 1, progress_type)
        log.debug(f"Get restore type: {restore_type}, log restore: {log_restore}")
        ret, mysql_data_dir = self.get_mysql_storage_path()
        if not ret:
            return False
        if not self.prepare_restore(restore_type, mysql_data_dir, copy_path):
            return False
        if not self.call_xtrabackup(restore_type, copy_path, mysql_data_dir):
            return False
        if not self.operate_file_power_after_restore(mysql_data_dir):
            return False
        self.replace_local_cert_file(restore_type, mysql_data_dir)
        if not self.restart_cluster(restore_type):
            return False
        if not MysqlServiceInfoUtil.wait_mysql_running(self._mysql_port):
            log.info(f"wait mysql running failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        self.wait_mysql_ready()
        # 同步证书文件
        self.save_cert_file(restore_type, mysql_data_dir)
        need_restore = log_restore and restore_type != RestoreType.PXC_INSTANCE_COMMON_NODE
        if need_restore and not self.log_restore(copy_path, restore_type):
            log.error(f"Exec log restore failed. pid:{self._p_id} jobId{self._job_id}")
            return False
        if not self.operate_master_node_in_ap_cluster(restore_type):
            return False
        if not self.operate_slave_node_in_ap_cluster(restore_type):
            return False
        log.info(f"Restore success. pid:{self._p_id} jobId:{self._job_id}")
        return True

    def save_cert_file(self, restore_type, mysql_data_dir):
        if restore_type != RestoreType.PXC_INSTANCE_CLUSTER_NODE:
            return
        target_path = os.path.join(self._cache_path, "cert")
        exec_mkdir_cmd(target_path, is_check_white_list=False)
        for file_name in os.listdir(mysql_data_dir):
            if not is_certificate_file(os.path.join(mysql_data_dir, file_name)):
                continue
            source_path = os.path.join(mysql_data_dir, file_name)
            log.info(f"source_path:{source_path},target_path:{target_path}")
            exec_cp_cmd(source_path, target_path, is_check_white_list=False)

    def replace_local_cert_file(self, restore_type, mysql_data_dir):
        if restore_type != RestoreType.PXC_INSTANCE_COMMON_NODE:
            return
        cert_dir = os.path.join(self._cache_path, "cert")
        for file_name in os.listdir(cert_dir):
            source_path = os.path.join(cert_dir, file_name)
            log.info(f"source_path:{source_path}")
            exec_cp_cmd(source_path, mysql_data_dir, is_check_white_list=False)
