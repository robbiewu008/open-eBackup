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

from common.common import execute_cmd
from common.util.cmd_utils import cmd_format
from openGauss.common.common import path_check, get_ids_by_name, check_injection_char, get_base_dir, join_path, \
    check_file_owner


class BaseCmd:

    def __init__(self, db_user, env_path):
        self.env_path = env_path
        self.db_user = db_user

    @staticmethod
    def _get_systemctl_status(serivce):
        if not check_injection_char(serivce):
            return False, "Invalid params to execute."
        status_cmd = f"systemctl status {serivce}"
        return_code, out_info, err_info = execute_cmd(status_cmd)
        ret = (return_code in ("0", "3"))
        res_cont = out_info if ret else err_info
        return ret, res_cont

    def execute_cmd(self, cmd: str):
        return self._get_dbtool_cmd_result(cmd)

    def check_user(self):
        try:
            uid, _ = get_ids_by_name(self.db_user)
        except Exception as e:
            return False
        return True

    def get_inst_port(self, data_path):
        port_cmd = "port"
        ret, cont = self._execute_guc_cmd(data_path, port_cmd)
        return ret, cont

    def get_query_info(self, data_path=None):
        query_cmd = "vb_ctl query"
        if data_path and check_injection_char(data_path):
            query_cmd = f"{query_cmd} -D {data_path}"
        ret, conn = self._get_dbtool_cmd_result(query_cmd)
        return ret, conn

    def get_db_version_info(self):
        ver_cmd = "gsql -V"
        ret, cont = self._get_dbtool_cmd_result(ver_cmd)
        return ret, cont

    def show_all_databases(self, data_port):
        sql_cmd = "select datname from pg_database;"
        ret, cont = self._execute_sql_cmd(sql_cmd, data_port)
        return ret, cont

    def show_archive_mode(self, data_port):
        sql_cmd = "show archive_mode;"
        ret, cont = self._execute_sql_cmd(sql_cmd, data_port)
        if not ret:
            return False
        if len(cont) > 0:
            split_lines = cont.splitlines()
            for index, row in enumerate(split_lines):
                if 1 < index < len(split_lines) - 1:
                    return row.strip() == "on"
        return False

    def get_control_data(self, data_path):
        if not check_injection_char(data_path):
            return False, "Invalid data path"
        sys_idt_cmd = f"pg_controldata {data_path}"
        ret, cont = self._get_dbtool_cmd_result(sys_idt_cmd)
        return ret, cont

    def get_sync_state(self, data_path):
        ret, cont = self._execute_guc_cmd(data_path, "synchronous_commit")
        return ret, cont

    def execute_sql_cmd(self, data_port, sql_cmd):
        ret, cont = self._execute_sql_cmd(sql_cmd, data_port)
        return ret, cont

    def _su_dbuser_prefix(self):
        if not check_injection_char(self.db_user) and not self.check_user():
            raise ValueError("Invalid user.")
        return cmd_format("su - {}", self.db_user)

    def _source_env_prefix(self):
        cmd = ""
        if not self.env_path:
            return cmd
        if path_check(self.env_path) and check_file_owner(self.env_path, self.db_user):
            cmd = f"source {self.env_path} &&"
            return cmd
        raise ValueError("Invalid env path")

    def _execute_sql_cmd(self, sql_cmd, db_port):
        if db_port:
            cmd = f"gsql -c \"{sql_cmd}\" postgres -p {db_port}"
        else:
            cmd = f"gsql -d postgres -c \"{sql_cmd}\""
        ret, cont = self._get_dbtool_cmd_result(cmd)
        return ret, cont

    def _execute_guc_cmd(self, data_path, param):
        if not check_injection_char(data_path) or not check_injection_char(param):
            return False, "Invalid params to execute cmd"
        port_cmd = f"LC_MESSAGES=\'en_US.UTF-8\' gs_guc check -D {data_path} -c {param}"
        ret, cont = self._get_dbtool_cmd_result(port_cmd)
        return ret, cont

    def _get_dbtool_cmd_result(self, dbtool_cmd):
        try:
            su = self._su_dbuser_prefix()
        except ValueError as usage:
            return False, str(usage)
        try:
            source_env = self._source_env_prefix()
        except ValueError as usage:
            return False, str(usage)
        ex_cmd = f"{su} -c '{source_env} {dbtool_cmd}'"
        return_code, out_info, err_info = execute_cmd(ex_cmd)
        ret = (return_code == "0")
        res_cont = out_info if ret else err_info
        return ret, res_cont


class GaussCmd(BaseCmd):

    def __init__(self, db_user, env_path):
        super().__init__(db_user, env_path)

    def get_view_info(self):
        gsview_cmd = "gs_om -t view"
        ret, cont = self._get_dbtool_cmd_result(gsview_cmd)
        return ret, cont

    def get_status(self):
        status_cmd = "gs_om -t status"
        ret, cont = self._get_dbtool_cmd_result(status_cmd)
        return ret, cont

    def get_status_all(self):
        status_cmd = "gs_om -t status --all"
        ret, cont = self._get_dbtool_cmd_result(status_cmd)
        return ret, cont

    def get_status_detail(self):
        status_cmd = "gs_om -t status --detail"
        ret, cont = self._get_dbtool_cmd_result(status_cmd)
        return ret, cont

    def get_status_by_name(self, node_name):
        status_cmd = f'gs_om -t status -h {node_name}'
        ret, cont = self._get_dbtool_cmd_result(status_cmd)
        return ret, cont

    def get_status_by_group_name(self, group_name):
        status_cmd = f'gs_om -t status --group-name {group_name} --detail'
        ret, cont = self._get_dbtool_cmd_result(status_cmd)
        return ret, cont

    def get_cmdb_status(self, address, port):
        query_cmd_status_command = cmd_format("ha_ctl monitor dcs -l http://{}:{}", address, port)
        ret, cont = self._get_dbtool_cmd_result(query_cmd_status_command)
        return ret, cont

    def check_cmdb_user(self, user, password, port):
        query_check_cmdb_user = \
            f'gsql -d postgres -U {user} -W {password} -p {port} -c \"select datname from pg_database;\"' if port else \
                f'gsql -d postgres -U {user} -W {password} -c \"select datname from pg_database;\"'
        ret, cont = self._get_dbtool_cmd_result(query_check_cmdb_user)
        return ret, cont


class VastBaseCmd(BaseCmd):
    def __init__(self, db_user, env_path):
        super().__init__(db_user, env_path)

    def get_status(self, data_path=None):
        status_cmd = "vb_ctl status"
        if data_path and check_injection_char(data_path):
            status_cmd = f"{status_cmd} -D {data_path}"
        ret, conn = self._get_dbtool_cmd_result(status_cmd)
        return ret, conn

    def get_data_path(self):
        data_path_cmd = "echo $PGDATA"
        ret, conn = self._get_dbtool_cmd_result(data_path_cmd)
        return ret, conn

    def get_has_service(self):
        ret, res_cont = self._get_systemctl_status("has")
        return ret, res_cont

    def get_has_start_argv(self):
        cmd = "systemctl show has -p ExecStart"
        return_code, out_info, err_info = execute_cmd(cmd)
        ret = (return_code in ("0", "3"))
        res_cont = out_info if ret else err_info
        return ret, res_cont

    def get_dcs_service(self):
        ret, res_cont = self._get_systemctl_status("dcs")
        return ret, res_cont

    def get_hasctl_cont(self, bin_path, etc_path):
        if not path_check(bin_path) or not path_check(etc_path):
            return False, ""
        hasctl_base_path = get_base_dir(bin_path)
        if not hasctl_base_path:
            return False, ""
        hasctl_path = join_path(hasctl_base_path, "hasctl")
        if not path_check(hasctl_path):
            return False, ""
        show_has_cont_cmd = f"{hasctl_path} -c {etc_path} list"
        ret, cont = self._get_dbtool_cmd_result(show_has_cont_cmd)
        return ret, cont

    def get_replcon_info(self, data_path, rep):
        ret, cont = self._execute_guc_cmd(data_path, rep)
        return ret, cont
