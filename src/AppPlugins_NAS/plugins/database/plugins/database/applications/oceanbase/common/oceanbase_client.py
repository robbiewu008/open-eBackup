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

import json
import pwd

import pexpect

from common.cleaner import clear
from oceanbase.common.const import CmdRetCode, OceanBaseResourceKeyName, ErrorCode
from oceanbase.common.oceanbase_common import get_env_variable, parse_sql_result, check_special_characters
from oceanbase.logger import log


class OceanBaseClient(object):

    def __init__(self, pid: str, param):
        log.info(f'client init begin, pid: {pid}')
        self.pid = pid
        self.param = param
        self.process_name = "obclient"
        self.process_owner = "admin"
        self.user = "root"
        self.tenant = "sys"
        self.db_user = ''
        self.db_pwd = ''
        self.timeout = 10
        if self.param.get('application', ''):
            self.app_info = self.param.get('application')
        else:
            self.app_info = self.param.get("applications", {})[0]
        self.cluster_name = self.app_info.get("name", "")
        self.app_extend_info = self.app_info.get('extendInfo', '')
        self.cluster_info_str = self.app_extend_info.get("clusterInfo", "")
        self.cluster_info = json.loads(self.cluster_info_str)
        self.cluster_id = self.cluster_info.get("clusterId", "")
        self.ob_proxy_ip = self.cluster_info.get('obProxyIp', '')
        self.ob_proxy_port = self.cluster_info.get('obProxyPort', 2883)
        self.observers = self.cluster_info.get('obServerAgents', '')
        self.observer_ip = self.observers[0].get('ip', '')
        self.observer_port = self.observers[0].get('port', 2881)
        self.obclients = self.cluster_info.get('obClientAgents', '')
        self.obclient_ip = self.obclients[0].get('ip', '')
        log.info(f'client init end, pid: {self.pid}')

    @staticmethod
    def exec_sql_cmd_pass(db_pwd, conn_cmd, exec_cmd, timeout=-1):
        process = pexpect.spawn(conn_cmd, encoding='utf-8')
        index = process.expect([pexpect.EOF, pexpect.TIMEOUT, "Enter password:"])
        if index in (0, 1):
            log.error(f"exec_sql_cmd_pass index: {index} OceanBase server connect failed.")
            return ErrorCode.ERR_DB_SERVICES, "", "OceanBase server connect failed."
        process.sendline(db_pwd)
        exec_ret_index = process.expect([pexpect.EOF, pexpect.TIMEOUT, "ERROR 2002", "ERROR 1045", "obclient"],
                                        timeout=timeout)
        if exec_ret_index in (0, 1, 2):
            log.error(f"exec_ret_index: {exec_ret_index} OceanBase server connect failed!")
            return ErrorCode.ERR_DB_SERVICES, "", "OceanBase server connect failed!"
        if exec_ret_index == 3:
            log.error(f"exec_ret_index: {exec_ret_index} Auth error")
            return ErrorCode.ERROR_AUTH, "", "Auth error!"
        log.info("success login")
        cmd_result = process.before
        if exec_cmd:
            process.sendline(exec_cmd)
            exec_ret_index = process.expect([pexpect.EOF, pexpect.TIMEOUT, 'obclient'])
            cmd_result = process.before
            log.info(f"exec_ret_index: {exec_ret_index} cmd_result: {cmd_result}")
            process.close()
            if exec_ret_index == 2:
                return CmdRetCode.EXEC_SUCCESS.value, cmd_result, ""
            if exec_ret_index in (0, 1):
                return CmdRetCode.EXEC_ERROR.value, "", "Exec sql cmd failed."
        else:
            return CmdRetCode.EXEC_SUCCESS.value, cmd_result, ""
        return CmdRetCode.EXEC_ERROR.value, "", f"Exec sql cmd {exec_cmd} failed."

    def exec_oceanbase_cmd(self, observer_ip, observer_port, sql_cmd, timeout=-1):
        try:
            pwd.getpwnam(self.process_owner)
        except KeyError:
            log.error(f"Param os user name {self.process_owner} error, pid: {self.pid}.")
            return CmdRetCode.EXEC_ERROR.value, "", f"Param os user name {self.process_owner} error."
        user, db_pwd = self.get_auth_from_param()
        if not check_special_characters(user):
            log.error(f'the user is Illegal {user}')
            raise Exception(f'the user is Illegal {user}')
        conn_cmd = f"obclient -h{observer_ip} -P{observer_port} -u{user}@{self.tenant} -p -A"
        log.debug(f"Exec conn_cmd: {conn_cmd} sql cmd: {sql_cmd}, pid: {self.pid}.")
        try:
            ret, out, err = self.exec_sql_cmd_pass(db_pwd, conn_cmd, sql_cmd, timeout=timeout)
        except Exception as ex:
            log.error(f"Spawn except an exception {ex}.")
            return CmdRetCode.EXEC_ERROR.value, "", f"Exec conn_cmd {conn_cmd} sql cmd {sql_cmd} failed."
        finally:
            clear(db_pwd)
        log.debug(f"Exec sql cmd result: {ret},{out},{err}, pid: {self.pid}.")
        return ret, out, err

    def get_auth_from_param(self):
        log.info("deal with auth")
        user = get_env_variable(f'{OceanBaseResourceKeyName.APPENV_AUTH_AUTHKEY}{self.pid}')
        db_pwd = get_env_variable(f'{OceanBaseResourceKeyName.APPENV_AUTH_AUTHPWD}{self.pid}')
        if not user or not db_pwd:
            user = get_env_variable(f'{OceanBaseResourceKeyName.LIST_APPLICATION_AUTH_AUTHKEY}{self.pid}')
            db_pwd = get_env_variable(f'{OceanBaseResourceKeyName.LIST_APPLICATION_AUTH_AUTHPWD}{self.pid}')

        if not user or not db_pwd:
            user = get_env_variable(f'{OceanBaseResourceKeyName.APPLICATION_AUTH_AUTHKEY}{self.pid}')
            db_pwd = get_env_variable(f'{OceanBaseResourceKeyName.APPLICATION_AUTH_AUTHPWD}{self.pid}')

        if not user or not db_pwd:
            user = get_env_variable(f'{OceanBaseResourceKeyName.JOB_TARGETENV_AUTH_AUTHKEY}{self.pid}')
            db_pwd = get_env_variable(f'{OceanBaseResourceKeyName.JOB_TARGETENV_AUTH_AUTHPWD}{self.pid}')

        return user, db_pwd

    def query_cluster_version(self):
        cmd = "show variables like '%version_comment%';"
        ret, out, err = self.exec_oceanbase_cmd(self.observer_ip, self.observer_port, cmd, timeout=self.timeout)
        version = ""
        if out and "Empty set" not in out:
            tmp = parse_sql_result(out)
            for item in tmp:
                if item.get('Value', ''):
                    version = item['Value'].split(' ')[1][0:5]
        log.info(f'verison {version}')
        return version

    def query_cluster_info(self, observer_ip, observer_port):
        cmd = "SELECT cluster_id,cluster_name,cluster_role,cluster_status FROM oceanbase.v$ob_cluster;"
        ret, out, err = self.exec_oceanbase_cmd(observer_ip, observer_port, cmd, timeout=self.timeout)
        clusters = []
        if out and "Empty set" not in out:
            clusters = parse_sql_result(out)
        log.info(f"clusters {clusters}")
        return clusters

    def query_observer(self, observer_ip, observer_port):
        cmd = "SELECT  id as server_id,svr_ip,svr_port,stop_time,status FROM oceanbase.__all_server;"
        ret, out, err = self.exec_oceanbase_cmd(observer_ip, observer_port, cmd, timeout=self.timeout)
        servers = []
        if out and "Empty set" not in out:
            servers = parse_sql_result(out)
            log.info(f"servers {servers}")
        return servers

    def query_tenant(self, observer_ip, observer_port):
        cmd = "SELECT tenant_name as name FROM oceanbase.gv$tenant;"
        ret, out, err = self.exec_oceanbase_cmd(observer_ip, observer_port, cmd, timeout=self.timeout)
        tenants = []
        if out and "Empty set" not in out:
            tmp = parse_sql_result(out)
            for item in tmp:
                if item['name'] == 'sys':
                    tmp.remove(item)
            tenants = tmp
        log.info(f'tenants {tenants}')
        return tenants

    def query_resource_pool(self, observer_ip, observer_port):
        cmd = "SELECT resource_pool_id,resource_pool_name from oceanbase.gv$unit where tenant_id is NULL group by " \
              "resource_pool_id, resource_pool_name;"
        ret, out, err = self.exec_oceanbase_cmd(observer_ip, observer_port, cmd, timeout=self.timeout)
        resource_pool = []
        if out and "Empty set" not in out:
            resource_pool = parse_sql_result(out)
        log.info(f"resource_pool {resource_pool}")
        return resource_pool

    def query_resource_pool_back(self, resource_pool_id=None):
        cmd = "SELECT * FROM oceanbase.__all_resource_pool"
        if resource_pool_id is not None:
            cmd = cmd + f" where resource_pool_id = '{resource_pool_id}';"
        ret, out, err = self.exec_oceanbase_cmd(self.observer_ip, self.observer_port, cmd, timeout=self.timeout)
        resource_pools = []
        dict_info = {}
        if out:
            result_list = out.strip().split('\r\n')[3:]
            for info in result_list:
                if '|' in info:
                    value_list = info.split('|')
                    dict_info = {
                        "resource_pool_id": value_list[3].strip(),
                        "name": value_list[4].strip(),
                        "unit_count": value_list[5].strip(),
                        "unit_config_id": value_list[6].strip(),
                        "zone_list": value_list[7].strip(),
                        "tenant_id": value_list[8].strip()
                    }
            resource_pools.append(dict_info)
        return resource_pools

    def query_restore_concurrency(self):
        cmd = "SHOW PARAMETERS LIKE 'restore_concurrency';"
        ret, out, err = self.exec_oceanbase_cmd(self.observer_ip, self.observer_port, cmd, timeout=self.timeout)
        restore_concurrencys = []
        dict_info = {}
        if out:
            result_list = out.strip().split('\r\n')[3:]
            for info in result_list:
                if '|' in info:
                    value_list = info.split('|')
                    dict_info = {
                        "value": value_list[7].strip()
                    }
            restore_concurrencys.append(dict_info)
        return restore_concurrencys[0].get('value')

    def execute_restore(self, dest_tenant_name, source_tenant_name, uri, **kwargs):
        timestamp = kwargs.get('timestamp')
        backup_cluster_name = kwargs.get('backup_cluster_name')
        backup_cluster_id = kwargs.get('backup_cluster_id')
        pool_list = kwargs.get('pool_list')
        restore_tables = kwargs.get('restore_tables')
        if restore_tables is not None:
            cmd = f"ALTER SYSTEM RESTORE {restore_tables} FOR {dest_tenant_name} FROM {source_tenant_name} at " \
                  f"'file://{uri}'"
        else:
            cmd = f"ALTER SYSTEM RESTORE {dest_tenant_name} FROM {source_tenant_name} at 'file://{uri}'"
        if timestamp is not None:
            cmd = cmd + f" until '{timestamp}'"
        cmd = cmd + f" with 'backup_cluster_name={backup_cluster_name}&backup_cluster_id={backup_cluster_id}&" \
                    f"pool_list={pool_list}';"
        return self.exec_oceanbase_cmd(self.observer_ip, self.observer_port, cmd, timeout=self.timeout)

    def query_max_restore_job_id(self):
        cmd = "SELECT MAX(JOB_ID) FROM oceanbase.CDB_OB_RESTORE_HISTORY;"
        ret, out, err = self.exec_oceanbase_cmd(self.observer_ip, self.observer_port, cmd, timeout=self.timeout)
        max_restore_job_id = 0
        if out and "Empty set" not in out and "NULL" not in out:
            max_restore_job_id = parse_sql_result(out)[0].get("MAX(JOB_ID)")
        return max_restore_job_id

    def query_restore_job(self, tenant_name=None, backup_tenant_name=None, max_restore_job_id=None):
        cmd = "SELECT JOB_ID,STATUS FROM oceanbase.CDB_OB_RESTORE_HISTORY where 1=1 "
        if tenant_name is not None:
            cmd = cmd + f" and TENANT_NAME = '{tenant_name}'"
        if backup_tenant_name is not None:
            cmd = cmd + f" and BACKUP_TENANT_NAME = '{backup_tenant_name}'"
        if max_restore_job_id is not None:
            cmd = cmd + f" and JOB_ID > {max_restore_job_id}"
        cmd = cmd + ";"
        ret, out, err = self.exec_oceanbase_cmd(self.observer_ip, self.observer_port, cmd, timeout=self.timeout)
        restore_jobs = None
        if out and "Empty set" not in out:
            restore_jobs = parse_sql_result(out)
        return restore_jobs

    def query_error_info(self, job_id):
        cmd = f"SELECT INFO FROM oceanbase.CDB_OB_RESTORE_HISTORY where JOB_ID = '{job_id}';"
        ret, out, err = self.exec_oceanbase_cmd(self.observer_ip, self.observer_port, cmd, timeout=self.timeout)
        restore_jobs = None
        if out and "Empty set" not in out and "NULL" not in out:
            restore_jobs = parse_sql_result(out)
        return restore_jobs

    def query_tenant_back(self, tenant_name=None):
        cmd = "SELECT tenant_name as name FROM oceanbase.gv$tenant"
        if tenant_name is not None:
            cmd = cmd + f" where TENANT_NAME = '{tenant_name}'"
        cmd = cmd + ";"
        ret, out, err = self.exec_oceanbase_cmd(self.observer_ip, self.observer_port, cmd, timeout=self.timeout)
        tenants = []
        if out and "Empty set" not in out:
            tmp = parse_sql_result(out)
            for item in tmp:
                if item['name'] == 'sys':
                    tmp.remove(item)
            tenants = tmp
        log.info(f'tenants {tenants}')
        return tenants
