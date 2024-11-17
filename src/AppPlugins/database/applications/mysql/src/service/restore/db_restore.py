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

from common.const import CMDResultInt
from common.exception.common_exception import ErrCodeException
from common.file_common import copy_user_file_to_dest_path, get_user_info, exec_lchown_dir_recursively, delete_file, \
    delete_path
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import exec_overwrite_file
from mysql import log
from mysql.src.common.constant import MysqlPxcStrictMode, ForbiddenStrictModeOptions
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.service.backup.backup_func import get_backup_tool_name, exec_xtrabackup_cmd
from mysql.src.service.restore.db_restore_func import drop_database, create_database, \
    get_dump_file, generate_discard_sql, generate_import_sql, restart_mysql, \
    wait_mysql_ready, get_tables_from_database, get_data_dir_user, set_foreign_key_checks, \
    get_pxc_strict_mode, set_pxc_strict_mode, generate_rename_sql
from mysql.src.service.restore.instance_restore import InstanceRestore
from mysql.src.service.restore.log_restore import LogRestore
from mysql.src.service.restore.restore_param import RestoreParam
from mysql.src.utils.common_func import get_version_from_sql, exec_cmd_spawn, source_sql_script
from mysql.src.utils.mysql_service_info_utils import MysqlServiceInfoUtil


class DBRestore(InstanceRestore):

    def __init__(self, job_id, sub_job_id, restore_param: RestoreParam):
        super().__init__(job_id, sub_job_id, restore_param)
        self.param: RestoreParam = restore_param
        self.origin_strict_mode = ""

    def set_origin_strict_mode(self, origin_strict_mode: str):
        self.origin_strict_mode = origin_strict_mode

    def operator_grastate_in_line(self, file_read):
        file_data = ""
        for line in file_read.readlines():
            if "seqno" in line:
                seqno = line
                num = int(seqno[seqno.find(':') + 1:].strip())
                if self.param.is_active_node:
                    num_new = num + 1
                else:
                    num_new = -1
                line = line.replace(str(num), str(num_new))
            if "safe_to_bootstrap" in line:
                bootstrap = line
                num = int(bootstrap[bootstrap.find(':') + 1:].strip())
                if self.param.is_active_node:
                    num_new = 1
                else:
                    num_new = 0
                line = line.replace(str(num), str(num_new))
            file_data += line
        log.info(f"Get grastate info: {file_data}. {self.get_log_comm()}")
        return file_data

    def modify_grastate_file(self):
        if not self.param.is_pxc_node:
            return
        grastate_info_file = os.path.join(self.param.data_dir, "grastate.dat")
        if not os.path.exists(grastate_info_file):
            log.warning(f"grastate.dat file not exist.{self.get_log_comm()}")
            return
        with open(grastate_info_file, 'r', encoding='utf-8') as file_read:
            file_data = self.operator_grastate_in_line(file_read)
        exec_overwrite_file(grastate_info_file, file_data, json_flag=False)

    def restart_mysql_waiting_running(self):
        self.modify_grastate_file()
        ret = restart_mysql(self.param.is_pxc_master, self.param.service_name, self.param.service_type,
                            self.param.my_cnf_path, self.param.port)
        if not ret:
            log.error(f"restart mysql error, {self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        if not MysqlServiceInfoUtil.wait_mysql_running(self.param.port):
            log.info(f"wait mysql running failed.{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        wait_mysql_ready(self.param.sql_param)

    def exec_restore(self):
        self.restart_mysql_waiting_running()
        set_fk_ret = set_foreign_key_checks(self.param.sql_param, "0")
        if self.param.is_pxc_node:
            self.check_pxc_strict_mode()
        if not self.param.is_restore_table_space:
            log.info(f"pxc slave do not operate table space,{self.get_log_comm()}")
            if set_fk_ret:
                set_foreign_key_checks(self.param.sql_param, "1")
        else:
            self.do_exec_restore(set_fk_ret)

    def check_pxc_strict_mode(self):
        strict_mode_str = get_pxc_strict_mode(self.param.sql_param)
        if not strict_mode_str:
            log.warning(f"Param forbiddenStrictMode invalid.pid:{self.get_log_comm()}")
            return
        if self.param.forbidden_strict_mode != ForbiddenStrictModeOptions.AllowStrictMode:
            return
        log.info(f"self.param.forbidden_strict_mode:{self.param.forbidden_strict_mode}")
        if strict_mode_str not in [MysqlPxcStrictMode.DISABLED, MysqlPxcStrictMode.PERMISSIVE]:
            log.info("strict_mode_str not in [DISABLED, PERMISSIVE]")
            raise ErrCodeException(MySQLErrorCode.ERR_PXC_STRICT_MODE)

    def do_exec_restore(self, set_fk_ret):
        log.info(f"db restore start,{self.get_log_comm()}")
        self.operate_slave_node_in_ap_cluster_before()
        self.delete_databases(self.param.get_copy_database)
        if self.param.target_database:
            self.delete_databases(self.param.target_database)
        if self.param.is_active_node:
            self.prepare_data()
        self.create_copy_database()
        self.generate_source_sql()
        self.source_discard_sql()
        self.copy_data_to_datadir()
        self.source_import_sql()
        if set_fk_ret:
            set_foreign_key_checks(self.param.sql_param, "1")
        if self.param.restore_time_stamp:
            log_restore = LogRestore(self.job_id, self.sub_job_id, self.param, self.param.get_copy_database)
            log_restore.exec_restore()
        self.rename_database()
        self.restart_mysql_waiting_running()
        self.operate_master_node_in_ap_cluster()
        self.operate_slave_node_in_ap_cluster_after()
        log.info(f"db restore end,{self.get_log_comm()}")

    def delete_databases(self, database):
        ret, output = drop_database(self.param.sql_param, database)
        if ret:
            log.info(f"delete database {database} success, {self.get_log_comm()}")
            return
        log.warning(f"delete database sql,output:{output} ,{self.get_log_comm()}")
        database_dir = os.path.join(self.param.data_dir, database)
        delete_path(database_dir)
        ret, output = drop_database(self.param.sql_param, database)
        if ret:
            log.info(f"retry delete database {database} success, {self.get_log_comm()}")
            return
        log.error(f"delete database error, {self.get_log_comm()}")
        raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)

    def create_copy_database(self):
        database = self.param.get_copy_database
        ret, output = create_database(self.param.sql_param, database)
        if not ret:
            log.error(f"create databases sql error,output:{output} ,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        ret, dump_file_path = get_dump_file(self.param.restore_copy_path, database)
        if not ret:
            log.error(f"get import structure file error,output:{output} ,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        cmd_str = cmd_format("mysql -u{} -h{} -P{} -p {} -e 'set sql_log_bin=0; source {};set sql_log_bin=1'",
                             self.param.sql_param.user, self.param.sql_param.host, self.param.port,
                             self.param.get_copy_database, dump_file_path)
        ret, output = exec_cmd_spawn(cmd_str, self.param.sql_param.passwd)
        if ret != CMDResultInt.SUCCESS.value:
            log.error(f"import dump file error,output:{output} ,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)

    def generate_source_sql(self):
        database = self.param.get_copy_database
        table_names = get_tables_from_database(self.param.sql_param, self.param.get_copy_database)
        if not table_names:
            log.warning(f"no tables in database,{database},{self.get_log_comm()}")
            log.info(f"create copy database database, {self.get_log_comm()}")
            return
        discard_sql_path = os.path.join(self.param.data_dir, "discard.sql")
        delete_file(discard_sql_path)
        log.info(f"discard_sql_path:{discard_sql_path}")
        generate_discard_sql(self.param.sql_param, database, discard_sql_path)
        import_sql_path = os.path.join(self.param.data_dir, "import.sql")
        log.info(f"import_sql_path:{import_sql_path}")
        delete_file(import_sql_path)
        generate_import_sql(self.param.sql_param, database, import_sql_path)

    def prepare_data(self):
        backup_tool = get_backup_tool_name(get_version_from_sql(self.param.sql_param))
        prepare_cmd = f"--prepare {self.param.get_force_recovery} --export --parallel={self.param.channel_number} " \
                      f"--target-dir={self.param.restore_copy_path}"
        log.info(f"prepare_cmd:{prepare_cmd}")
        ret, output = exec_xtrabackup_cmd(prepare_cmd, backup_tool)
        if not ret:
            # mariadb prepare可能会报错不影响
            log.warning(f"exec prepare cmd failed. output:{output} {self.get_log_comm()}")
        log.info(f"prepare data success {self.get_log_comm()}")

    def copy_data_to_datadir(self):
        user_name = get_data_dir_user(self.param.data_dir)
        user_group, user_info = get_user_info(user_name)
        target_path = os.path.join(self.param.data_dir, self.param.get_copy_database)
        src_parent = os.path.join(self.param.restore_copy_path, self.param.get_copy_database)
        for file_name in os.listdir(src_parent):
            log.info(f"file_name:{file_name}")
            src_path = os.path.join(src_parent, file_name)
            if file_name.endswith("ibd"):
                log.info(f"copyfile to data dir, src_path:{src_path}, target_path:{target_path}")
                copy_user_file_to_dest_path(src_path, target_path)
        exec_lchown_dir_recursively(self.param.data_dir, user_name, user_group)

    def source_discard_sql(self):
        discard_sql_path = os.path.join(self.param.data_dir, "discard.sql")
        if not os.path.exists(discard_sql_path):
            log.warning("discard sql path not exists")
            return
        self.set_pxc_strict_mode()
        ret, output = source_sql_script(self.param.sql_param, discard_sql_path)
        if not ret:
            log.error(f"source discard sql,output:{output} ,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        delete_file(discard_sql_path)
        log.info(f"source discard sql success,{self.get_log_comm()}")

    def source_import_sql(self):
        import_sql_path = os.path.join(self.param.data_dir, "import.sql")
        if not os.path.exists(import_sql_path):
            log.warning("import sql path not exists")
            return
        ret, output = source_sql_script(self.param.sql_param, import_sql_path)
        if not ret:
            log.error(f"source import sql,output:{output} ,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        delete_file(import_sql_path)
        self.restore_pxc_strict_mode()
        log.info(f"source import sql success,{self.get_log_comm()}")

    def set_pxc_strict_mode(self):
        if not self.param.is_pxc_node:
            return
        pxc_mode_str = get_pxc_strict_mode(self.param.sql_param)
        if not pxc_mode_str:
            log.error(f"get pxc strict mode failed,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        log.info(f"pxc_mode_str:{pxc_mode_str}")
        if pxc_mode_str not in [MysqlPxcStrictMode.MASTER, MysqlPxcStrictMode.ENFORCING]:
            return
        self.set_origin_strict_mode(pxc_mode_str)
        ret, output = set_pxc_strict_mode(self.param.sql_param, MysqlPxcStrictMode.DISABLED)
        if not ret:
            log.error(f"set pxc mode error:{output} ,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)

    def restore_pxc_strict_mode(self):
        if not self.param.is_pxc_node:
            return
        if not self.origin_strict_mode:
            return
        ret, output = set_pxc_strict_mode(self.param.sql_param, self.origin_strict_mode)
        if not ret:
            log.error(f"set pxc mode error:{output} ,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)

    def rename_database(self):
        if not self.param.newname_database:
            log.info("no need to rename database")
            return
        create_database(self.param.sql_param, self.param.newname_database)
        table_names = get_tables_from_database(self.param.sql_param, self.param.get_copy_database)
        if not table_names:
            log.info(f"database is empty, no need to rename database. {self.get_log_comm()}")
            return
        rename_sql_path = os.path.join(self.param.data_dir, "rename.sql")
        delete_file(rename_sql_path)
        generate_rename_sql(self.param.sql_param, self.param.get_copy_database, self.param.newname_database,
                            rename_sql_path)
        ret, output = source_sql_script(self.param.sql_param, rename_sql_path)
        if not ret:
            log.error(f"source rename sql,output:{output} ,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        delete_file(rename_sql_path)
        log.info(f"rename database {self.param.get_copy_database} to {self.param.newname_database} success")
        self.delete_databases(self.param.get_copy_database)
