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

import fileinput
import os

from mysql import log
from common.common import execute_cmd, check_command_injection, execute_cmd_list, execute_cmd_list_with_communicate
from common.file_common import get_user_info, exec_lchown_dirs_recursively
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import su_exec_rm_cmd
from mysql.src.common.constant import ExecCmdResult, MySQLJsonConstant, RestoreType, MysqlStatusStr, MySQLParamType, \
    MariaDBNeedExcludeDir, MYSQL_SERVICE_LIST, MysqlBackupToolName
from mysql.src.common.execute_cmd import check_mysql_command_injection, mysql_get_status_output
from mysql.src.utils.mysql_utils import MysqlUtils

G_NEED_PERPARE_MYSQL_TYPE = [
    # 单实例或者集群实例的主节点需要去做一次perpare
    RestoreType.SINGLE_DB,
    RestoreType.AP_DB_CLUSTER_MASTER_NODE,
    RestoreType.PXC_DB_CLUSTER_NODE,
    RestoreType.SINGLE_INSTANCE,
    RestoreType.AP_INSTANCE_CLUSTER_MASTER_NODE,
    RestoreType.PXC_INSTANCE_CLUSTER_NODE,
    RestoreType.EAPP_INSTANCE
]


class MysqlRestoreCommon:
    def __init__(self, p_id, job_id, sub_job_id, json_param):
        self._p_id = p_id
        self._job_id = job_id
        self._sub_job_id = sub_job_id
        self._json_param = json_param
        self._result_file_array = []

    @staticmethod
    def check_selinux_status_off():
        ret, _, _ = execute_cmd_list(["getenforce", "grep Disabled"], True)
        if ret == ExecCmdResult.SUCCESS:
            return True
        return False

    @staticmethod
    def check_selinux_install_status():
        try:
            ret, _, _ = execute_cmd("getenforce")
        except Exception as exception_str:
            log.warning("The SELinux is not installed on this system.")
            return True
        log.info("The SELinux is installed on this system.")
        return False

    @staticmethod
    def operate_chcon_contest(mysql_data_dir):
        """
        开启selinux模式，需要修改安全上下文
        :return:
        """
        # 针对linux系统没有安装selinux情况下，不做selinux相关操作（SUSE默认不会安装selinux）
        if MysqlRestoreCommon.check_selinux_install_status():
            return True
        if MysqlRestoreCommon.check_selinux_status_off():
            return True
        if check_mysql_command_injection(mysql_data_dir):
            log.error("Command injection.")
            return False
        cmd_str = cmd_format("chcon -Rv -u system_u -t mysqld_db_t {}", mysql_data_dir)
        log.info("Selinux mode is enabled.")
        ret, output = mysql_get_status_output(cmd_str)
        if ret != int(ExecCmdResult.SUCCESS):
            log.error("Exec chcon mysql failed.")
            return False
        return True

    @staticmethod
    def get_copy_database_name(copy_path):
        copy_database_name = ""
        for path in os.listdir(copy_path):
            if path == MariaDBNeedExcludeDir.ROCKSDB or \
                    path.startswith(MariaDBNeedExcludeDir.HASGTAG):
                log.info(f"Exclude dir {path}.")
                continue
            new_path = os.path.join(copy_path, path)
            if os.path.isdir(new_path):
                copy_database_name = path
                break
        if not copy_database_name:
            return False, ""
        log.info(f"Get copy database name: {copy_database_name}")
        return True, copy_database_name

    @staticmethod
    def rename_and_recoverable(rename_database_name, copy_database_name, dir_names):
        """
        数据库恢复重命名的判断
        """
        # 重命名为自己/选择的数据库
        if rename_database_name == copy_database_name:
            return True
        # 重命名为其他名，且不存在重命名的数据库
        if rename_database_name not in dir_names:
            return True
        return False

    @staticmethod
    def rename_new_pos_and_recoverable(rename_database_name, copy_database_name, dir_names):
        """
        新位置数据库恢复重命名的判断
        """
        # 存在同名数据库，恢复失败
        if copy_database_name in dir_names:
            return False
        # 重命名为自己/选择的数据库
        if rename_database_name == copy_database_name:
            return True
        # 重命名为其他名，且不存在重命名的数据库
        if rename_database_name not in dir_names:
            return True
        return False

    @staticmethod
    def original_location_and_recoverable(copy_database_name, choose_database_name, dir_names):
        """
        数据库恢复未重命名的判断
        """
        # 备份数据库和下发数据库同名
        if copy_database_name == choose_database_name:
            return True
        # 新位置，当前环境不存在同副本同名的数据库
        if copy_database_name not in dir_names:
            return True
        return False

    @staticmethod
    def check_mysql_can_not_restore():
        """
        查询mysql状态，是否能做恢复,不能做返回True
        :return: 
        """
        # 只有处于inactive(dead) 和failed状态的MYSQL才能做恢复
        for mysql_service in MYSQL_SERVICE_LIST:
            # 命令硬编码可以使用shell=false的接口
            ret, _, _ = execute_cmd_list_with_communicate(["systemctl -all", f"grep -w {mysql_service}"])
            if ret != ExecCmdResult.SUCCESS:
                continue
            ret, output, _ = execute_cmd(f"systemctl status {mysql_service}")
            if not output:
                continue
            if MysqlStatusStr.STOP not in output and MysqlStatusStr.FAILED not in output:
                log.error(f"Mysql service {mysql_service} status error.")
                return True

        return False

    def parse_new_database_name(self):
        """
        获取界面重名名参数
        """
        job_json = self._json_param.get(MySQLJsonConstant.JOB, {})
        if not job_json:
            log.warning(f"Get job json failed. pid:{self._p_id} jobId{self._job_id}")
            return ""
        extend_info_json = job_json.get(MySQLJsonConstant.EXTENDINFO, {})
        if not extend_info_json:
            log.warning(f"Get extend info failed. pid:{self._p_id} jobId{self._job_id}")
            return ""
        new_database_name = extend_info_json.get(MySQLJsonConstant.NEW_DATABASE_NAME, "")
        if not new_database_name:
            log.warning(f"Get new database name failed. pid:{self._p_id} jobId{self._job_id}")
            return ""
        log.debug(f"Get new database name： {new_database_name}")
        return new_database_name

    def rename_restore(self, choose_database_name, copy_database_name, rename_database_name, dir_names):
        """
        数据库恢复重命名的判断
        """
        # 原位置
        if copy_database_name == choose_database_name and self.rename_and_recoverable(rename_database_name,
                                                                                      copy_database_name, dir_names):
            return True
        # 新位置
        if copy_database_name != choose_database_name and self.choose_name_and_copy_name_restore(
                choose_database_name, copy_database_name, rename_database_name, dir_names):
            return True
        return False

    def get_restore_type_is_database(self):
        """
        获取是否为数据库恢复
        """
        sub_type = self._json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.TARGETOBJECT, {}).get(
            MySQLJsonConstant.SUBTYPE, "")
        if not sub_type:
            log.error(f"Get sub type json failed. pid:{self._p_id} jobId{self._job_id}")
            return False
        log.info(f"Get sub type: {sub_type}. pid:{self._p_id} jobId{self._job_id}")
        if sub_type != MySQLParamType.DATABASE:
            return False
        log.info(f"Get sub type is database: {sub_type}. pid:{self._p_id} jobId{self._job_id}")
        return True

    def choose_name_and_copy_name_restore(self, choose_database_name, copy_database_name, rename_database_name,
                                          dir_names):
        """
        数据库恢复新位置下重命名选择数据库的判断
        """
        # 选择同副本同名的数据库
        if choose_database_name == copy_database_name and self.rename_and_recoverable(rename_database_name,
                                                                                      copy_database_name, dir_names):
            return True
        # 选择不同名的数据库
        if choose_database_name != copy_database_name and self.rename_new_pos_and_recoverable(rename_database_name,
                                                                                              copy_database_name,
                                                                                              dir_names):
            return True
        return False

    """
    mysql可以配置在数据目录之外的表空间文件，恢复时，需要将数据目录之外的通用表空间文件找到，删除掉才能恢复
    以下二个函数实现相关功能：
    1.get_isl_file_list_by_copy():从副本中获取所有.isl的文件类型列表 .isl文件记录了真正的表空间文件在哪里
    2.deal_ibd_files():循环读取isl文件列表，找到所有的ibd文件的真正地址，判断如果对应的文件存在 就删除掉
    """
    def get_isl_file_list_by_copy(self, copy_path):
        """
        从副本中获取所有.isl的文件类型列表 .isl文件记录了真正的表空间文件在哪里
        return:所有isl_list的列表
        """
        isl_list = []
        for root, _, files in os.walk(copy_path, topdown=False):
            for file in [os.path.join(root, f) for f in files]:
                if os.path.isfile(file) and file.endswith(".isl"):
                    isl_list.append(file)
        log.info(f"Get isl file success.count({len(isl_list)}). pid:{self._p_id} jobId{self._job_id}")
        return isl_list

    def deal_ibd_files(self, isl_list, my_cnf_path):
        """
        遍历所有isl文件中记录的真正文件路径
        su - mysql -s /bin/bash -c "rm -f xxx"的方式进行删除，可以不对文件进行校验
        shell命令有长度限制，考虑效率的情况下，一次删除100个以内的文件
        """

        def read_files(index):
            try:
                with fileinput.input(isl_list[index:index + 100]) as file_op:
                    for line in file_op:
                        file_result_array.append(line.strip())
            except Exception as exception_str:
                log.error(f"Read isl file failed. pid:{self._p_id} jobId{self._job_id},error:{exception_str}")
                return False
            return True

        user_name = MysqlUtils.get_data_path_user_name(my_cnf_path)
        for index in range(0, len(isl_list), 100):
            file_result_array = []
            if not read_files(index):
                return False
            self._result_file_array.append(file_result_array)

            for path in file_result_array:
                ret = su_exec_rm_cmd(path, user_name, check_white_black_list_flag=False)
                if ret:
                    continue
                log.error(f"Rm file[{path}] failed. pid:{self._p_id} jobId{self._job_id},user_name:{user_name}")
                ret = su_exec_rm_cmd(path, user_name=None, check_white_black_list_flag=False)
                if not ret:
                    log.error(f"Rm file[{path}] failed. pid:{self._p_id} jobId{self._job_id}")
                    return False
        return True

    def move_one_ibd_file(self, ibd_file, mysql_data_dir, database_name):
        """
        通过ibd_file，将数据库目录下的指定ibd文件，拷贝到对应的位置
        """
        source_file = os.path.join(mysql_data_dir, database_name, os.path.basename(ibd_file))
        if os.path.exists(source_file):
            try:
                os.rename(source_file, ibd_file)
            except Exception as exception_str:
                log.error(f"Rename failed. exception({exception_str}).pid:{self._p_id} jobId{self._job_id}")
                return False
        return True

    def move_ibd_files(self, mysql_data_dir, database_name):
        """
        将配置在数据目录之外的通用表空间文件都移动到对应的位置
        """
        for file_array in self._result_file_array:
            for idb_file in file_array:
                if not self.move_one_ibd_file(idb_file, mysql_data_dir, database_name):
                    return False
        return True

    def get_result_file_array(self):
        return self._result_file_array

    def chown_ibd_files(self, my_cnf_path):
        """
        恢复完成之后，修改所有通用表空间文件的用户和属组
        """
        user_name = MysqlUtils.get_data_path_user_name(my_cnf_path)
        user_group, user_info = get_user_info(user_name)
        for file_array in self._result_file_array:
            if file_array and not exec_lchown_dirs_recursively(file_array, user_name, user_group):
                log.error(f"Exec chown ibd file failed. pid:{self._p_id} jobId:{self._job_id}")
                return False
        log.info(f"Chown ibd file success. pid:{self._p_id} jobId:{self._job_id}")
        return True

    def operate_chcon_contest_ibd(self):
        """
        开启selinux模式，需要修改安全上下文
        :return:
        """
        for file_array in self._result_file_array:
            file_str = ' '.join(file_array)
            if not self.operate_chcon_contest(file_str):
                log.error(f"Chcon ibd file failed. pid:{self._p_id} jobId:{self._job_id}")
                return False
        log.info(f"Chcon ibd file success. pid:{self._p_id} jobId:{self._job_id}")
        return True

    def get_force_recover_str(self):
        force_recovery = self.get_force_recovery()
        if force_recovery == 1:
            # 该选项的作用是强制恢复，可以在数据库页损坏的情况下，将数据强制恢复
            return "--innodb-force-recovery=1"
        return ""

    def get_force_recovery(self):
        """
        从参数中获取是否为强制恢复
        """
        force_recovery = 0
        extend_info = self._json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.EXTENDINFO, {})
        if not extend_info:
            log.warning(f"Job extendInfo failed. pid:{self._p_id} jobId{self._job_id}")
            return force_recovery
        force_recovery = int(extend_info.get(MySQLJsonConstant.FORCE_RECOVERY, 0))
        log.info(f"Get force recovery success({force_recovery}).")
        return force_recovery

    def get_is_forbidden_strict_mode(self):
        """
        从参数中获取是否为禁止严格模式 0 不禁止 1 禁止
        """
        forbidden_strict_mode = 0
        extend_info = self._json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.EXTENDINFO, {})
        if not extend_info:
            log.warning(f"Job extendInfo failed. pid:{self._p_id} jobId{self._job_id}")
            return forbidden_strict_mode
        forbidden_strict_mode = int(extend_info.get(MySQLJsonConstant.FORBIDDEN_STRICT_MODE, 0))
        log.info(f"Get forbidden strict mode success({forbidden_strict_mode}).")
        return forbidden_strict_mode

    def deal_xtrabackup_error(self, output, tool_name):
        """
        处理命令的错误输出
        """
        if tool_name not in [MysqlBackupToolName.XTRBACKUP2, MysqlBackupToolName.XTRBACKUP8]:
            return
        try:
            for err_flag in ["[FATAL]", "Error:"]:
                index = output.find(err_flag)
                if index != -1:
                    log.error(f"CMD error:{output[index:]}, job id:{self._job_id}")
                    return
        except Exception as except_str:
            log.error(f"Deal cmd error failed.,{except_str}")
