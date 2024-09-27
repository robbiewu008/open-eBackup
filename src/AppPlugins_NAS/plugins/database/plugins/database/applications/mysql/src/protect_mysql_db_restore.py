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
import re

from common.cleaner import clear
from common.common import exter_attack, check_command_injection, is_clone_file_system
from common.const import SubJobStatusEnum
from common.util.check_utils import check_repo_path
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import su_exec_rm_cmd, exec_overwrite_file, exec_mv_cmd, exec_cp_cmd
from mysql import log
from mysql.src.common.constant import ForbiddenStrictModeOptions, XtrbackupErrStr
from mysql.src.common.constant import MySQLStrConstant, RestorePath, MySQLRestoreStep, MysqlBackupToolName, \
    MysqlExecSqlError, MysqlPxcStrictMode
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.execute_cmd import exec_sql, exec_sql_without_binlog
from mysql.src.common.execute_cmd import safe_get_environ
from mysql.src.protect_mysql_base import RestoreType
from mysql.src.protect_mysql_restore_base import MysqlRestoreBase
from mysql.src.protect_mysql_restore_common import MysqlRestoreCommon, G_NEED_PERPARE_MYSQL_TYPE
from mysql.src.protect_mysql_undo_service import MysqlUndoService
from mysql.src.utils.mysql_service_info_utils import MysqlServiceInfoUtil


class MysqlDatabaseRestore(MysqlRestoreBase):
    def __init__(self, p_id, job_id, sub_job_id, json_param):
        super().__init__(p_id, job_id, sub_job_id, json_param)
        self._duplicate_database_name = ""
        self._replica_database_name = ""
        self._copy_data_name = ""
        self._restore_type = 0
        self._pxc_strict_mode = ""
        self._error_code = 0
        self._restore_step = 0

    def get_file_total(self, copy_path):
        file_total = [
            "xtrabackup_info", "xtrabackup_tablespaces", "xtrabackup_checkpoints", "xtrabackup_binlog_pos_innodb",
            "xtrabackup_master_key_id", "xtrabackup_galera_info",
            "backup-my.cnf", "xtrabackup_logfile", self._copy_data_name + ".sql"
        ]
        for file_name in os.listdir(copy_path):
            if file_name.startswith("ibdata"):
                file_total.insert(0, file_name)
        log.info(f"file_total:{file_total}")
        return file_total

    def delete_old_files(self, file_total, mysql_data_dir):
        """
        数据库恢复需要删除当前数据库中，ibdata、xtrabackup_info、xtrabackup_tablespaces、xtrabackup_checkpoints、
        xtrabackup_binlog_pos_innodb、xtrabackup_master_key_id、xtrabackup_galera_info和数据库目录等相关的以.old结尾的文件目录
        """
        for file in os.listdir(mysql_data_dir):
            if not file.endswith(RestorePath.DOTOLD):
                continue
            file_name_main, file_name_ext = os.path.splitext(file)
            # 在file_total表中 或 任务id含有4个'-',才进行删除 或者undo开头的
            if (os.path.isfile(file) and file_name_main not in file_total) or file.count('-') != 4:
                continue
            if file.startswith(MySQLStrConstant.UNDO_PREFIX):
                continue
            path_file = os.path.join(mysql_data_dir, file)
            if os.path.islink(path_file):
                log.warning(f"Path has link.")
                continue
            if not su_exec_rm_cmd(path_file, check_white_black_list_flag=False):
                raise Exception(f"The path[{path_file}] delete failed.")

    def delete_and_change_files_in_copy(self, copy_path):
        """
        数据库恢复需要删除克隆副本中，除ibdata、xtrabackup_info、xtrabackup_tablespaces、xtrabackup_checkpoints、undo***
        xtrabackup_binlog_pos_innodb、xtrabackup_master_key_id、xtrabackup_galera_info和数据库目录外的文件
        """
        file_total = self.get_file_total(copy_path)
        # 恢复出的数据库目录文件改名为 ：任务ID名_数据库名.old
        file_change_name = self._copy_data_name
        for sub_file in os.listdir(copy_path):
            if sub_file in file_total:
                continue
            if sub_file.startswith(MySQLStrConstant.UNDO_PREFIX):
                continue
            sub_file_path = os.path.join(copy_path, sub_file)
            if sub_file == file_change_name:
                # 由于8.x版本，需要在启数据库前将恢复出的数据库目录中的文件改名，不然后续的导表操作会失败
                for sub_file1 in os.listdir(sub_file_path):
                    sub_filed_path = os.path.join(sub_file_path, sub_file1)
                    os.rename(sub_filed_path, sub_filed_path + RestorePath.DOTOLD)
                change_name = os.path.join(copy_path, f"{self._job_id}{self._copy_data_name}{RestorePath.DOTOLD}")
                os.rename(sub_file_path, change_name)
                continue
            # 集群下已改名的数据库目录，需要保留
            if sub_file == f"{self._job_id}{self._copy_data_name}{RestorePath.DOTOLD}":
                continue
            # 之前mysql的相关删除处理，同安全讨论过，可不添加操作
            if not su_exec_rm_cmd(sub_file_path, check_white_black_list_flag=False):
                raise Exception(f"The path[{sub_file_path}] delete failed.")

    def call_xtrabackup(self, restore_type, copy_path, mysql_data_dir):
        if self._restore_step >= MySQLRestoreStep.XTRABACKUP_RESTORE:
            log.debug(f"Step {MySQLRestoreStep.XTRABACKUP_RESTORE} has done. pid:{self._p_id} jobId:{self._job_id}")
            return True
        ret, my_cnf_path = self.find_mycnf_path(self.my_cnf_path)
        if not ret:
            log.error(f"Find my.cnf failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        channel_number = self.get_channel_number()
        tool_name = self.get_backup_tool_name()
        if restore_type in G_NEED_PERPARE_MYSQL_TYPE:
            log.info(f"Start prepare. pid:{self._p_id} jobId:{self._job_id}")
            # mysql5.7 8.0需要再恢复前执行prepare做一次未提交事务的回滚
            parper_cmd_param = cmd_format("--prepare {} --export --parallel={} --target-dir={}",
                                          self._restore_comm.get_force_recover_str(), channel_number, copy_path)
            ret, output = self.exec_xtrabackup_cmd(parper_cmd_param, tool_name)
            # mariadb 执行prepared可能会报错，也可能不会报错，但是mariadb不需要prepared，所以直接忽略
            if not ret:
                if self._internal_error == XtrbackupErrStr.DATABASE_COPY_IS_PREPARED:
                    log.warn(f"Exec prepare cmd failed. but is prepared. pid:{self._p_id} jobId:{self._job_id}")
                else:
                    log.error(f"Exec prepare cmd failed. output:{output} pid:{self._p_id} jobId:{self._job_id}")
                    return False
        passwd_str = ""
        try:
            if tool_name == MysqlBackupToolName.MARIADBBACKUP:
                passwd_str = f"={safe_get_environ(self._mysql_pwd)}"
            # 删除副本文件中不需要的文件
            self.delete_and_change_files_in_copy(copy_path)
            restore_cmd_param = cmd_format("--defaults-file={} --host={} --user={} --password'{}' --port={} "
                                           "--datadir={} {} --parallel={} --force-non-empty-directories "
                                           "--copy-back --target-dir={}", my_cnf_path, self._mysql_ip,
                                           self._mysql_user, passwd_str, self._mysql_port, mysql_data_dir,
                                           self._restore_comm.get_force_recover_str(), channel_number, copy_path)
            MysqlUndoService.delete_undo_files(self._job_id, self.my_cnf_path)
            ret, output = self.exec_xtrabackup_cmd(restore_cmd_param, tool_name, self._mysql_pwd)
            if not ret:
                log.error(f"Exec restore cmd failed. pid:{self._p_id} jobId:{self._job_id}")
                return False
        finally:
            clear(passwd_str)
        log.info(f"Exec restore cmd success. pid:{self._p_id} jobId:{self._job_id}")
        ret = self.ib_data_handle(mysql_data_dir)
        if not ret:
            return False
        # 把备份的undo文件恢复回去
        MysqlUndoService.restore_undo_files(self._job_id, self.my_cnf_path)
        self.write_restore_step_info(MySQLRestoreStep.XTRABACKUP_RESTORE)
        return True

    def ib_data_handle(self, mysql_data_dir):
        # 由于恢复会把此前备份副本中的ibdata一起恢复回来，所以先删除它，再把之前改名的ibdata.old改回来，再起数据库
        for file_name in os.listdir(mysql_data_dir):
            log.info(f"ib_data_handle file_name:{file_name}")
            if not file_name.startswith("ibdata"):
                continue
            if file_name.endswith(".tmp"):
                continue
            ib_data = os.path.join(mysql_data_dir, file_name)
            if not su_exec_rm_cmd(ib_data, check_white_black_list_flag=False):
                log.error(f"Rm file failed. path:[{ib_data}]. pid:{self._p_id} jobId:{self._job_id}")
                return False
            exec_mv_cmd(f"{ib_data}.tmp", ib_data, check_white_black_list_flag=False)
        return True

    def operator_grastate_in_line(self, file_read, restore_type):
        """
        文件类容，需要修改含有seqno和safe_to_bootstrap两行的内容
        seqno：主节点的修改往上加1，保证最大，备节点改为-1，保证全量同步
        safe_to_bootstrap：主节点设为1，备节点设为2
        """
        file_data = ""
        for line in file_read.readlines():
            if "seqno" in line:
                seqno = line
                num = int(seqno[seqno.find(':') + 1:].strip())
                if restore_type == RestoreType.PXC_DB_CLUSTER_NODE:
                    num_new = num + 1
                else:
                    num_new = -1
                line = line.replace(str(num), str(num_new))
            if "safe_to_bootstrap" in line:
                bootstrap = line
                num = int(bootstrap[bootstrap.find(':') + 1:].strip())
                if restore_type == RestoreType.PXC_DB_CLUSTER_NODE:
                    num_new = 1
                else:
                    num_new = 0
                line = line.replace(str(num), str(num_new))
            file_data += line
        log.debug(f"Get grastate info: {file_data}. pid:{self._p_id} jobId{self._job_id}")
        return file_data

    def read_and_modify_grastate_file(self, grastate_info_file, restore_type):
        # pxc集群节点操作grastate文件，此文件数据库如果破坏，会自己重新生成。此外由于mysql高切低有问题，之前已同安全测试沟通过。
        file_data = ""
        try:
            with open(grastate_info_file, 'r', encoding='utf-8') as file_read:
                file_data = self.operator_grastate_in_line(file_read, restore_type)
        except Exception as exception_str:
            log.error(f"Replace value in grastate failed. pid:{self._p_id} jobId{self._job_id}")
            return False
        if not file_data:
            log.error(f"Get value in grastate failed. pid:{self._p_id} jobId{self._job_id}")
            return False

        ret = exec_overwrite_file(grastate_info_file, file_data, json_flag=False)
        if not ret:
            log.error(f"Write value in grastate failed. pid:{self._p_id} jobId{self._job_id}")
            return False
        return True

    def modify_grastate_file(self, mysql_data_dir, restore_type):
        """
        读取并修改grastate.dat文件，只有PXC集群需要此修改
        """
        if restore_type == RestoreType.PXC_DB_COMMON_NODE or restore_type == RestoreType.PXC_DB_CLUSTER_NODE:
            log.info(f"Get restore type: {restore_type} in modify grastate file. pid:{self._p_id} jobId{self._job_id}")
            grastate_info_file = os.path.join(mysql_data_dir, "grastate.dat")
            if not self.read_and_modify_grastate_file(grastate_info_file, restore_type):
                return False
        return True

    def failed_rename_database_same_name_local_database(self, copy_database_name, mysql_data_dir, param_database_name):
        """
        如果改名数据库，其恢复副本中的数据库名与本地同名，不支持恢复
        :return:
        """
        mysql_restore_common = MysqlRestoreCommon(self._p_id, self._job_id, self._sub_job_id, self._json_param)
        dst_database_name = mysql_restore_common.parse_new_database_name()
        if not dst_database_name and copy_database_name == param_database_name:
            return True
        if dst_database_name == copy_database_name:
            return True
        self._duplicate_database_name = dst_database_name
        self._replica_database_name = param_database_name
        # 原位置/新位置是同名的数据库，重命名打开，可以恢复
        if copy_database_name == param_database_name:
            log.info(
                f"Restore the database with the same name and then rename the database. pid:{self._p_id} "
                f"jobId:{self._job_id}")
            return True
        for dirs_files in os.listdir(mysql_data_dir):
            temp_dirs_files = os.path.join(mysql_data_dir, dirs_files)
            if os.path.exists(temp_dirs_files) and os.path.isdir(temp_dirs_files) and dirs_files == copy_database_name:
                log.error(
                    f"Duplicate database names, but the database names in the backup copy are the same as those in the "
                    f"local database. pid:{self._p_id} jobId:{self._job_id}")
                return False
        return True

    def rename_files_from_original_database_dir(self, copy_path, mysql_data_dir, restore_type):
        """
        数据库恢复前需要移走以下文件才能恢复
        ibdata1 xtrabackup_info xtrabackup_tablespaces(8.x版本会需要) 及相关数据库名
        """
        # 从副本中获取数据库名
        ret, database_name = MysqlRestoreCommon.get_copy_database_name(copy_path)
        if not ret:
            log.error(f"Get copy database name failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        if self._job_id not in database_name:
            self._copy_data_name = database_name
        else:
            # 恢复出的数据库目录文件改名为 ：任务ID名_数据库名.old,这里去掉最后4位.old,并拿到数据库名
            sub_dir_name = database_name[:-4]
            end_index = [id_name.end() for id_name in re.finditer(self._job_id, sub_dir_name)]
            if not end_index:
                log.error(f"Get database name empty from change dir failed. pid:{self._p_id} jobId:{self._job_id}")
                return False
            self._copy_data_name = sub_dir_name[end_index[0]:]
        log.info(f"Get real copy database name: {self._copy_data_name}, pid:{self._p_id} jobId:{self._job_id}")

        if self._restore_step >= MySQLRestoreStep.RENAME_BEFORE_RESTORE:
            log.debug(f"Step {MySQLRestoreStep.RENAME_BEFORE_RESTORE} has done. pid:{self._p_id} jobId:{self._job_id}")
            return True

        ret, param_database_name = self.get_restore_database()
        # param_database_name：原位置即为原备份数据库名，新位置为界面选的对应节点的数据库名（不是重命名的名字）
        log.info(f"Get param database name: {param_database_name}")
        if not ret:
            log.error(f"Exec get restore database failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        if not self.failed_rename_database_same_name_local_database(self._copy_data_name, mysql_data_dir,
                                                                    param_database_name):
            return False

        file_total = self.get_file_total(copy_path)
        self.delete_old_files(file_total, mysql_data_dir)
        # 数据库需要原来ibdata1文件启，所以这里保留并改名后缀.tmp,待恢复命令成功后，把恢复出的此文件删除，再把改名的文件改回
        for file_name in os.listdir(mysql_data_dir):
            log.info(f"file_name:{file_name}")
            if not file_name.startswith("ibdata"):
                continue
            if file_name.endswith(".tmp"):
                continue
            ibdata_file = os.path.join(mysql_data_dir, file_name)
            if not os.path.exists(f"{ibdata_file}.tmp"):
                exec_mv_cmd(ibdata_file, f"{ibdata_file}.tmp", check_white_black_list_flag=False)
        for file in file_total:
            src_file = os.path.join(mysql_data_dir, file)
            exec_mv_cmd(src_file, f"{src_file}{RestorePath.DOTOLD}", check_white_black_list_flag=False)

        # 备份undo文件
        MysqlUndoService.backup_undo_files(self._job_id, self.my_cnf_path)
        self.write_restore_step_info(MySQLRestoreStep.RENAME_BEFORE_RESTORE)
        return True

    def clear_database_dir(self, mysql_data_dir, database_name):
        database_dir = os.path.join(mysql_data_dir, database_name)
        try:
            self.clean_dir(database_dir)
        except Exception as exception_str:
            log.error(f"Clear database({database_name}) dir failed. pid:{self._p_id} jobId{self._job_id}")
            return False
        log.info(f"Clear database({database_name}) dir success. pid:{self._p_id} jobId{self._job_id}")
        return True

    def clean_data_base(self, mysql_data_dir):
        # 需要清空数据库目录才能删除数据库。
        if not self.clear_database_dir(mysql_data_dir, self._copy_data_name):
            return False
        cmd_str = cmd_format("drop database `{}`", self._copy_data_name)
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = cmd_str
        ret, output = exec_sql_without_binlog(exec_sql_param)
        if not ret:
            log.error(f"Exec_sql failed. sql:drop replica database({self._copy_data_name}), "
                      f"ret:{ret} pid:{self._p_id} jobId{self._job_id}")
            return False
        if self._duplicate_database_name and self._copy_data_name != self._replica_database_name:
            # 新位置重命名需要删除选择的数据库
            cmd_str = cmd_format("drop database `{}`", self._replica_database_name)
            exec_sql_param.sql_str = cmd_str
            ret, output = exec_sql_without_binlog(exec_sql_param)
            if not ret:
                log.error(f"Exec_sql failed. sql:drop replica database({self._replica_database_name}), "
                          f"ret:{ret} pid:{self._p_id} jobId{self._job_id}")
                return False
        return True

    def duplicate_database_name(self, mysql_data_dir, restore_type):
        if restore_type == RestoreType.PXC_DB_COMMON_NODE:
            return True
        duplicate_database_name = self._duplicate_database_name
        if not self._duplicate_database_name:
            duplicate_database_name = self._replica_database_name
            if not self._replica_database_name:
                return True

        exec_sql_param = self.generate_exec_sql_param()
        cmd_str = cmd_format("drop database if exists `{}`", duplicate_database_name)
        exec_sql_param.sql_str = cmd_str
        ret, output = exec_sql_without_binlog(exec_sql_param)
        if not ret:
            log.error(f"Exec sql failed. sql:drop duplicate database, ret:{ret} pid:{self._p_id}, jobId{self._job_id}")
            return False
        cmd_str = cmd_format("create database if not exists `{}`", duplicate_database_name)
        exec_sql_param.sql_str = cmd_str
        ret, output = exec_sql_without_binlog(exec_sql_param)
        if not ret:
            log.error(f"Exec sql failed. sql:create database, ret:{ret} pid:{self._p_id} "
                      f"jobId{self._job_id}")
            return False
        cmd_str = f"select table_name from information_schema.tables where table_schema='{self._copy_data_name}'"
        exec_sql_param.sql_str = cmd_str
        ret, output = exec_sql(exec_sql_param)
        if not ret:
            log.error(f"Exec sql failed. sql:select table name, ret:{ret} pid:{self._p_id} "
                      f"jobId{self._job_id}")
            return False
        for i in output:
            cmd_str = f"rename table `{self._copy_data_name}`.`{i[0]}` to `{duplicate_database_name}`.`{i[0]}`"
            log.info(f"cmd_str:{cmd_str}")
            exec_sql_param.sql_str = cmd_str
            ret, output = exec_sql_without_binlog(exec_sql_param)
            if not ret:
                log.error(f"Exec sql failed. sql:rename table, ret:{ret} pid:{self._p_id} "
                          f"jobId{self._job_id}")
                return False
        if not self.clean_data_base(mysql_data_dir):
            return False
        return True

    def find_table_names_new(self):
        exec_sql_param = self.generate_exec_sql_param()
        cmd_str = f"select table_name from information_schema.tables where table_schema='{self._copy_data_name}'"
        exec_sql_param.sql_str = cmd_str
        ret, output = exec_sql(exec_sql_param)
        log.info(f"ret:{ret},output:{output}")
        table_names = []
        for i in output:
            table_names.append(i[0])
        return table_names

    def get_pxc_strict_mode(self):
        pxc_mode_str = ""
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = "show variables like 'pxc_strict_mode'"
        ret, output = exec_sql(exec_sql_param)
        if not ret:
            log.error(f"Exec sql Get pxc strict mode failed. pid:{self._p_id} jobId:{self._job_id}")
            return False, pxc_mode_str
        for i in output:
            if len(i) < 2:
                continue
            if i[0] == "pxc_strict_mode":
                pxc_mode_str = i[1]
                break
        if not pxc_mode_str:
            log.error(f"Get pxc strict mode failed. pid:{self._p_id} jobId:{self._job_id}")
            return False, pxc_mode_str
        log.info(f"Get pxc strict mode success({pxc_mode_str}). pid:{self._p_id} jobId:{self._job_id}")
        return True, pxc_mode_str

    def set_pxc_strict_mode(self, mode_str):
        cmd_str = f"SET GLOBAL pxc_strict_mode = {mode_str}"
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = cmd_str
        ret, _ = exec_sql(exec_sql_param)
        if not ret:
            log.error(f"Exec sql Set pxc strict mode({mode_str}) failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        log.info(f"Set pxc strictm mode({mode_str}) success. pid:{self._p_id} jobId:{self._job_id}")
        return True

    def deal_pxc_strict_mode(self, discard):
        """
        PXC集群导入表空间，需要禁用PXC的严格模式
        """
        if self._restore_type not in [RestoreType.PXC_DB_CLUSTER_NODE,
                                      RestoreType.PXC_DB_COMMON_NODE]:
            return True
        if not discard:
            if self._pxc_strict_mode:
                return self.set_pxc_strict_mode(self._pxc_strict_mode)
            return True
        ret, pxc_mode_str = self.get_pxc_strict_mode()
        if not ret or not pxc_mode_str:
            return False
        if pxc_mode_str in [MysqlPxcStrictMode.MASTER, MysqlPxcStrictMode.ENFORCING]:
            ret = self.set_pxc_strict_mode(MysqlPxcStrictMode.DISABLED)
            if not ret:
                return False
            self._pxc_strict_mode = pxc_mode_str
        return True

    def alter_table_space(self, table_names, copy_database_name, discard):
        """
        表空间操作
        """
        if discard:
            ret = self.deal_pxc_strict_mode(discard)
            if not ret:
                log.error(f"Deal pxc strict mode failed. pid:{self._p_id} jobId{self._job_id}")
                return False
        for table in table_names:
            log.info(f"table name:{table}")
            if discard:
                cmd_str = f"ALTER TABLE `{table}` DISCARD TABLESPACE"
            else:
                cmd_str = f"ALTER TABLE `{table}` IMPORT TABLESPACE"
            try:
                exec_sql_param = self.generate_exec_sql_param()
                exec_sql_param.sql_str = cmd_str
                exec_sql_param.database_name = copy_database_name
                ret, output_err = exec_sql_without_binlog(exec_sql_param)
            except Exception as exception_str:
                # 数据库恢复，导入表空间失败时，需要上报错误码到界面
                if MysqlExecSqlError.ERROR_TABLE_NOT_FOUND in str(exception_str):
                    log.info(f"Import table space failed. pid:{self._p_id} jobId:{self._job_id}")
                    self._error_code = MySQLErrorCode.RESTORE_IMPORT_TABLE_SPACE_FAILED
                ret = False
            if not ret:
                log.error(f"Exec sql failed. sql:{table} discard: {discard} tablespace, ret:{ret} pid:{self._p_id} "
                          f"jobId:{self._job_id}")
                return False
        log.info(f"Exec sql success. sql: discard: {discard} tablespace, pid:{self._p_id} jobId:{self._job_id}")
        if not discard:
            ret = self.deal_pxc_strict_mode(discard)
            if not ret:
                log.error(f"Deal pxc strict mode failed. pid:{self._p_id} jobId{self._job_id}")
                return False
        return True

    def delete_export_file(self, database_dir, export_symbol):
        """
        删除由于备份添加--export生成的导出文件.exp和.cfg文件
        """
        for file in os.listdir(database_dir):
            file_path = os.path.join(database_dir, file)
            #  非文件、后缀index的两种情况，不满足情况
            if not os.path.isfile(file_path):
                continue
            file_name_main, file_name_ext = os.path.splitext(file)
            if file_name_ext != export_symbol:
                continue
            if not su_exec_rm_cmd(file_path, check_white_black_list_flag=False):
                raise Exception(f"The path[{file_path}] delete failed.")
        log.info(f"Delete {export_symbol} files. pid:{self._p_id} jobId:{self._job_id}")

    def import_table_structure(self, database_name, file_table_structure):
        """
        source sql文件，执行表结构的导入
        """
        if check_command_injection(file_table_structure):
            log.error(f"Check mysql command injection. pid:{self._p_id} jobId{self._job_id}")
            return False
        cmd_str = cmd_format("-u{} -h{} -P{} -p {} -e 'set sql_log_bin=0; source {};set sql_log_bin=1'",
                             self._mysql_user, self._mysql_ip, self._mysql_port, database_name, file_table_structure)
        ret, output = self.exec_xtrabackup_cmd(cmd_str, MysqlBackupToolName.MYSQL, self._mysql_pwd)
        if not ret:
            log.error(f"Exec import table structure cmd failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        log.info(f"Exec import table structure cmd success. pid:{self._p_id} jobId:{self._job_id}")
        return True

    def get_import_table_structure_file(self, copy_path, copy_database_name):
        """
        获取sql文件
        """
        file_table_structure = ""
        for path in os.listdir(copy_path):
            new_path = os.path.join(copy_path, path)
            if copy_database_name + ".sql" == path:
                file_table_structure = new_path
                break
        if not file_table_structure:
            log.error(f"Get file table structure failed. pid:{self._p_id}, jobId{self._job_id}")
            return False, ""
        log.info(f"Get file table structure success. pid:{self._p_id}, jobId{self._job_id}")
        return True, file_table_structure

    def drop_database(self, database_name):
        if check_command_injection(database_name):
            log.error(f"Check command injection. pid:{self._p_id}, jobId{self._job_id}")
            return False
        cmd_str = cmd_format("drop database if exists `{}`", database_name)
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = cmd_str
        ret, output = exec_sql_without_binlog(exec_sql_param)
        if not ret:
            log.error(f"Exec sql failed. sql:drop database, ret:{ret} pid:{self._p_id}, jobId{self._job_id}")
            return False
        log.info(f"Exec sql success. sql:drop database, ret:{ret} pid:{self._p_id}, jobId{self._job_id}")
        return True

    def create_database(self, database_name):
        if check_command_injection(database_name):
            log.error(f"Check command injection. pid:{self._p_id}, jobId{self._job_id}")
            return False
        cmd_str = cmd_format("create database if not exists `{}`", database_name)
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = cmd_str
        ret, output = exec_sql_without_binlog(exec_sql_param)
        if not ret:
            log.error(f"Exec_sql failed. sql:create database, ret:{ret} pid:{self._p_id} jobId{self._job_id}")
            return False
        log.info(f"Exec sql success. sql:create database, ret:{ret} pid:{self._p_id}")
        return True

    def mv_files_copy_database_name_dir(self, mysql_data_dir, database_name):
        """
        mv恢复出的数据库目录（前面已改为任务id数据库名。old）文件到备份副本同名的目录路径下
        """
        database_dir = os.path.join(mysql_data_dir, database_name)
        change_database_name_dir = os.path.join(mysql_data_dir,
                                                f"{self._job_id}{self._copy_data_name}{RestorePath.DOTOLD}")
        for sub_file in os.listdir(change_database_name_dir):
            sub_file_path = os.path.join(change_database_name_dir, sub_file)
            if sub_file == "db.opt":
                if not su_exec_rm_cmd(sub_file_path, check_white_black_list_flag=False):
                    log.error(f"Rm file failed. path:[{sub_file_path}]. pid:{self._p_id} jobId:{self._job_id}")
                continue
            # 去掉最后4位.old
            dst_path = os.path.join(database_dir, sub_file[:-4])
            file_name_main, file_name_ext = os.path.splitext(sub_file)
            # 需要对.ibd .cfg .exp结尾的文件改名，不然执行下面import出错
            if file_name_ext == ".frm":
                continue
            exec_mv_cmd(sub_file_path, dst_path, check_white_black_list_flag=False)

    def delete_database(self, mysql_data_dir):
        """
        删除备份副本同名的目录
        """
        database_dir = os.path.join(mysql_data_dir, self._copy_data_name)
        if not os.path.exists(database_dir):
            return True
        cmd_str = f"select table_name from information_schema.tables where table_schema='{self._copy_data_name}'"
        if check_command_injection(self._copy_data_name):
            log.error(f"Check command injection. pid:{self._p_id}, jobId{self._job_id}")
            return False
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = cmd_str
        ret, output = exec_sql(exec_sql_param)
        if not ret:
            log.error(f"Exec sql failed. sql:select table name, ret:{ret} pid:{self._p_id} jobId{self._job_id}")
            return False
        log.info(f"Exec sql success. sql:select table name, pid:{self._p_id} jobId{self._job_id}")
        for i in output:
            cmd_str = f"drop table if exists `{self._copy_data_name}`.`{i[0]}`"
            exec_sql_param.sql_str = cmd_str
            ret, output = exec_sql_without_binlog(exec_sql_param)
            if not ret:
                log.error(f"Exec sql failed. sql:drop table {i[0]}, ret:{ret} pid:{self._p_id} jobId{self._job_id}")
                return False
        log.info(f"Exec sql success. sql:drop table, pid:{self._p_id} jobId{self._job_id}")
        # 删除残余文件
        for sub_file in os.listdir(database_dir):
            sub_file_dir = os.path.join(database_dir, sub_file)
            if not su_exec_rm_cmd(sub_file_dir, check_white_black_list_flag=False):
                log.error(f"Rm file failed. path:[{sub_file_dir}]. pid:{self._p_id} jobId:{self._job_id}")
        # 删除数据库
        if not self.drop_database(self._copy_data_name):
            return False
        log.info(f"Delete database dir success. ret:{ret} pid:{self._p_id}")
        return True

    def operate_table_space_ex(self, copy_path, mysql_data_dir, restore_type):
        set_foreign_key_checks_flag = False
        if not self.set_foreign_key_checks(0):
            log.warning(f"Set foreign key checks 0 failed. pid:{self._p_id} jobId{self._job_id}")
        else:
            set_foreign_key_checks_flag = True
        if self._restore_type in [RestoreType.PXC_DB_CLUSTER_NODE, RestoreType.PXC_DB_COMMON_NODE]:
            if not self.check_pxc_strict_mode():
                log.error(f"PXC strict mode not matched.pid:{self._p_id} jobId{self._job_id}")
                return False
        ret = self.operate_table_space(copy_path, mysql_data_dir, restore_type)
        if set_foreign_key_checks_flag and not self.set_foreign_key_checks(1):
            log.warning(f"Set foreign key checks 1 failed. pid:{self._p_id} jobId{self._job_id}")
        return ret

    def check_pxc_strict_mode(self):
        ret, pxc_strict_mode = self.get_pxc_strict_mode()
        if not ret or not pxc_strict_mode:
            log.warning(f"Param forbiddenStrictMode invalid.pid:{self._p_id} jobId{self._job_id}")
            return True
        mysql_restore_common = MysqlRestoreCommon(self._p_id, self._job_id, self._sub_job_id, self._json_param)
        is_forbidden_strict_mode = mysql_restore_common.get_is_forbidden_strict_mode()
        if is_forbidden_strict_mode not in \
                (ForbiddenStrictModeOptions.ForbiddenStrictMode, ForbiddenStrictModeOptions.AllowStrictMode):
            log.error(f"Param forbiddenStrictMode invalid.pid:{self._p_id} jobId{self._job_id}")
            return False
        if is_forbidden_strict_mode == ForbiddenStrictModeOptions.ForbiddenStrictMode:
            return True
        if is_forbidden_strict_mode == ForbiddenStrictModeOptions.AllowStrictMode:
            if pxc_strict_mode not in [MysqlPxcStrictMode.DISABLED, MysqlPxcStrictMode.PERMISSIVE]:
                self._error_code = MySQLErrorCode.ERR_PXC_STRICT_MODE
                return False
        return True

    def operate_database_chown_chcon(self, mysql_data_dir):
        data_dir = os.path.join(mysql_data_dir, self._copy_data_name)
        if not self.operate_chown(data_dir, False):
            return False
        if not self._restore_comm.operate_chcon_contest(mysql_data_dir):
            return False
        if not self._restore_comm.move_ibd_files(mysql_data_dir, self._copy_data_name):
            return False
        if not self._restore_comm.chown_ibd_files(self.my_cnf_path):
            return False
        if not self._restore_comm.operate_chcon_contest_ibd():
            return False
        return True

    def operate_table_space(self, copy_path, mysql_data_dir, restore_type):
        """
        数据库恢复操作表空间
        """
        if restore_type == RestoreType.PXC_DB_COMMON_NODE:
            return True
        if restore_type == RestoreType.AP_DB_CLUSTER_SLAVE_NODE:
            log.info(f"Get restore_type: {restore_type} equal AP DB CLUSTER SLAVE NODE")
            exec_sql_param = self.generate_exec_sql_param()
            exec_sql_param.sql_str = "stop slave"
            ret, output = exec_sql(exec_sql_param)
            if not ret:
                log.error(f"Exec failed, sql:stop slave. ret:{ret} pid:{self._p_id} jobId{self._job_id}")
                return False
        if not self.delete_database(mysql_data_dir):
            return False
        if not self.create_database(self._copy_data_name):
            return False
        change_database_dir = os.path.join(mysql_data_dir, f"{self._job_id}{self._copy_data_name}{RestorePath.DOTOLD}")
        ret, file_table_structure = self.get_import_table_structure_file(copy_path, self._copy_data_name)
        if not ret:
            return False
        if not self.import_table_structure(self._copy_data_name, file_table_structure):
            return False
        table_names = self.find_table_names_new()
        log.info(f"new table_names:{table_names}")
        if table_names and not self.alter_table_space(table_names, self._copy_data_name, True):
            return False
        self.mv_files_copy_database_name_dir(mysql_data_dir, self._copy_data_name)
        # 删除改名为id.old的目录
        if not su_exec_rm_cmd(change_database_dir, check_white_black_list_flag=False):
            log.error(f"Rm file failed. path:[{change_database_dir}]. pid:{self._p_id} jobId:{self._job_id}")
        if not self.operate_database_chown_chcon(mysql_data_dir):
            return False
        if table_names and not self.alter_table_space(table_names, self._copy_data_name, False):
            log.error(f"Alter import table space failed. pid:{self._p_id}, jobId{self._job_id}")
            return False
        database_dir = os.path.join(mysql_data_dir, self._copy_data_name)
        self.delete_export_file(database_dir, ".cfg")
        self.delete_export_file(database_dir, ".exp")
        return True

    def operate_start_service(self, mysql_data_dir, restore_type):
        # 修改undo文件夹下的undo文件权限
        if not MysqlUndoService.change_undo_files_permission(self._job_id, self.my_cnf_path):
            return False
        if not self.operate_chown(mysql_data_dir):
            return False
        if not MysqlRestoreCommon.operate_chcon_contest(mysql_data_dir):
            return False
        if not self.modify_grastate_file(mysql_data_dir, restore_type):
            return False
        if not self.restart_cluster(restore_type):
            return False
        return True

    def prepare_restore(self, restore_type, mysql_data_dir, copy_path):
        # 安全校验
        if is_clone_file_system(self.get_json_param()) and not check_repo_path(copy_path):
            log.error("Check db restore copy_path invalid.")
            return False
        if not self._restore_comm.deal_ibd_files(self._restore_comm.get_isl_file_list_by_copy(copy_path),
                                                 self.my_cnf_path):
            return False
        if not self.rename_files_from_original_database_dir(copy_path, mysql_data_dir, restore_type):
            return False
        return True

    def set_foreign_key_checks(self, key_close):
        """
        开启或者关闭Mysql外建约束检查
        key_close = 0：关闭
        key_close = 1：开启
        """
        cmd_str = f"SET GLOBAL FOREIGN_KEY_CHECKS={key_close}"
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = cmd_str
        ret, output = exec_sql(exec_sql_param)
        if not ret:
            log.error(f"Exec_sql failed. sql:{cmd_str} ret:{ret} pid:{self._p_id} jobId{self._job_id}")
            return False
        log.info(f"Set FOREIGN_KEY_CHECKS={key_close} success. pid:{self._p_id} jobId{self._job_id}")
        return True

    @exter_attack
    def exec_sub_job(self, restore_type, log_restore):
        if not self.check_mysql_cluster_is_stop():
            return False
        log.info(f"DB restore begin. pid:{self._p_id} jobId:{self._job_id}")
        self._restore_type = restore_type
        self._restore_step = self.read_restore_step_info()
        ret, copy_path = self.get_restore_copy_path()
        if not ret:
            log.error(f"Exec set copy path failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        if not is_clone_file_system(self.get_json_param()):
            data_tmp_dir = os.path.join("/tmp", self._job_id)
            exec_cp_cmd(copy_path, data_tmp_dir, is_check_white_list=False)
            copy_path = data_tmp_dir
        progress_type = self._sub_job_id if self._sub_job_id else self._job_id
        self.write_progress_file(SubJobStatusEnum.RUNNING.value, 1, progress_type)
        ret, mysql_data_dir = self.get_mysql_storage_path()
        if not ret:
            log.error(f"Get mysql storage path failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        if not self.prepare_restore(restore_type, mysql_data_dir, copy_path):
            return False
        if not self.call_xtrabackup(restore_type, copy_path, mysql_data_dir):
            return False
        # 修改属性、安全上下文、启数据库
        if not self.operate_start_service(mysql_data_dir, restore_type):
            return False
        # 等待启动成功
        if not MysqlServiceInfoUtil.wait_mysql_running(self._mysql_port):
            log.info(f"wait mysql running failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        log.info(f"mysql start success! pid:{self._p_id} jobId:{self._job_id}")
        self.wait_mysql_ready()
        if not self.operate_table_space_ex(copy_path, mysql_data_dir, restore_type):
            return False
        if log_restore:
            if not self.log_restore(copy_path, restore_type, self._copy_data_name):
                log.error(f"Exec log restore failed. pid:{self._p_id} jobId{self._job_id}")
                return False
        if not self.duplicate_database_name(mysql_data_dir, restore_type):
            return False
        if not self.operate_slave_node_in_ap_cluster(restore_type):
            return False
        if not self.operate_master_node_in_ap_cluster(restore_type):
            return False
        log.info(f"Restore success. pid:{self._p_id} jobId:{self._job_id}")
        return True
