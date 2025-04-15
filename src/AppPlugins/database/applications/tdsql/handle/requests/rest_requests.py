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
import time
import random

from common.file_common import log
from common.parse_parafile import get_env_variable
from tdsql.common.util import request_post
from tdsql.common.const import RestoreParam
from tdsql.handle.common.const import TDSQLProtectKeyName


class RestRequests:

    @staticmethod
    def build_extend_param(params, extend_info):
        if not extend_info.get("extParameter"):
            return params
        ext_parameter = json.loads(extend_info.get("extParameter"))
        log.info(f'start_restore_to_original ext_parameter : {ext_parameter}')
        if ext_parameter:
            if ext_parameter.get("machine"):
                params["machine"] = ext_parameter.get("machine")
            if ext_parameter.get("cpu"):
                params["cpu"] = str(int(float(ext_parameter.get("cpu")) * 100))
            if ext_parameter.get("memory"):
                params["memory"] = str(int(float(ext_parameter.get("memory")) * 1000))
            if ext_parameter.get("logDisk"):
                params["log_disk"] = str(int(float(ext_parameter.get("logDisk")) * 1000))
            if ext_parameter.get("dataDisk"):
                params["data_disk"] = str(int(float(ext_parameter.get("dataDisk")) * 1000))
        return params

    @staticmethod
    def get_user_passwd(env_variable):
        user = get_env_variable(env_variable)
        passwd = get_env_variable(env_variable)
        return user, passwd

    @staticmethod
    def get_assign_ip(node_ips):
        index = 0
        assign_ip = ""
        for node_ip in node_ips:
            index = index + 1
            assign_ip += node_ip
            if index != len(node_ips):
                assign_ip += ";"
        return assign_ip

    def start_restore_to_original(self, restore_param: RestoreParam):
        user, passwd = self.get_user_passwd(restore_param.env_variable)
        timestamp = int(time.time())
        node_num = int(restore_param.job_extend_info.get("drMode"))
        choose_nodes = random.sample(restore_param.node_ips, node_num)
        assign_ip = self.get_assign_ip(choose_nodes)
        log.info(f'start_restore_to_original assign_ip : {assign_ip}')
        params = {
            "groupid": restore_param.group_id,
            "retreatedtime": restore_param.restore_time,
            "nodeNum": node_num,
            "sync_type": 1,
            "idc_flag": False,
            "lessidc": False,
            "fast": False,
            "same_idc_upgrade": 0,
            "same_proxy": 1,
            "same_db": 1,
            "manual": True,
            "assign_ip": assign_ip
        }
        all_params = self.build_extend_param(params, restore_param.job_extend_info)
        log.info(f'start_restore_to_original all_params : {all_params}')
        interface = {
            "interfaceName": "TDSQL.RetreatGroup",
            "para": all_params
        }
        request_body = {
            "callee": "TDSQL",
            "caller": user,
            "eventId": 101,
            "interface": interface,
            "password": passwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        retry_nums = 0
        while retry_nums < 3:
            retry_nums += 1
            if retry_nums != 1:
                time.sleep(3)
            ret, ret_body, ret_header = request_post(restore_param.request_url, request_body, request_header)
            log.info(f'start_restore_to_original ret_body : {ret_body}, retry_nums:  {retry_nums}')
            if not ret:
                log.error(f'Failed start_restore_to_original, ret_body is : {ret_body}')
                continue
            if ret_body.get("returnMsg") != "ok":
                log.error(f'start_restore_to_original error with return: {ret_body.get("returnMsg")}')
                continue
            return ret_body.get("returnData").get("taskid")
        return -1

    def start_restore_to_new(self, restore_param: RestoreParam):
        user, passwd = self.get_user_passwd(restore_param.env_variable)
        timestamp = int(time.time())
        node_num = int(restore_param.job_extend_info.get("drMode"))
        choose_nodes = random.sample(restore_param.node_ips, node_num)
        assign_ip = self.get_assign_ip(choose_nodes)
        log.info(f'start_restore_to_new assign_ip : {assign_ip}')
        params = {
            "groupid": restore_param.group_id,
            "retreatedtime": restore_param.restore_time,
            "nodeNum": node_num,
            "sync_type": 1,
            "idc_flag": False,
            "lessidc": False,
            "fast": False,
            "same_idc_upgrade": 0,
            "manual": True,
            "assign_ip": assign_ip
        }
        all_params = self.build_extend_param(params, restore_param.job_extend_info)
        log.info(f'start_restore_to_new all_params : {all_params}')
        interface = {
            "interfaceName": "TDSQL.ClusterRetreatGroup",
            "para": all_params
        }
        request_body = {
            "callee": "TDSQL",
            "caller": user,
            "eventId": 101,
            "interface": interface,
            "password": passwd,
            "timestamp": timestamp,
            "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        retry_nums = 0
        while retry_nums < 3:
            retry_nums += 1
            if retry_nums != 1:
                time.sleep(3)
            ret, ret_body, ret_header = request_post(restore_param.request_url, request_body, request_header)
            log.info(f'start_restore_to_new ret_body : {ret_body}, retry_nums:  {retry_nums}')
            if not ret:
                log.error(f'Failed start_restore_to_new, ret_body is : {ret_body}')
                continue
            if ret_body.get("returnMsg") != "ok":
                log.error(f'start_restore_to_new error with return: {ret_body.get("returnMsg")}')
                continue
            return ret_body.get("returnData").get("taskid")
        return -1

    def query_restore_status_original(self, request_url, group_id, taskid, env_variable):
        user, passwd = self.get_user_passwd(env_variable)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.QueryGroupRetreat",
                "para": {
                    "groupid": group_id,
                    "taskid": taskid
                }
            },
            "password": passwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        retry_nums = 0
        while retry_nums < 3:
            retry_nums += 1
            if retry_nums != 1:
                time.sleep(3)
            ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
            log.info(f'query_restore_status_original ret_body : {ret_body}, retry_nums:  {retry_nums}')
            if not ret:
                log.error(f'Failed query_restore_status_original, ret_body is : {ret_body}')
                continue
            if ret_body.get("returnMsg") != "ok":
                log.error(f'query_restore_status_original error with return: {ret_body.get("returnMsg")}')
                continue
            status = ret_body.get("returnData").get("status")
            task_step = ret_body.get("returnData").get("cur_step")
            err_msg = ret_body.get("returnData").get("err_msg")
            return status, task_step, err_msg
        return -1, "", ""

    def query_restore_status_new(self, request_url, taskid, env_variable):
        user, passwd = self.get_user_passwd(env_variable)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.QueryClusterRetreatGroup",
                "para": {
                    "taskid": taskid
                }
            },
            "password": passwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        retry_nums = 0
        while retry_nums < 3:
            retry_nums += 1
            if retry_nums != 1:
                time.sleep(3)
            ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
            log.info(f'query_restore_status_new ret_body : {ret_body}, retry_nums:  {retry_nums}')
            if not ret:
                log.error(f'Failed query_restore_status_new, ret_body is : {ret_body}')
                continue
            if ret_body.get("returnMsg") != "ok":
                log.error(f'query_restore_status_new error with return: {ret_body.get("returnMsg")}')
                continue
            status = ret_body.get("returnData").get("status")
            task_step = ret_body.get("returnData").get("cur_step")
            err_msg = ret_body.get("returnData").get("err_msg")
            return status, task_step, err_msg
        return -1, "", ""

    def query_db_version(self, request_url, env_variable):
        user, passwd = self.get_user_passwd(env_variable)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.GetDbVersion",
                "para": {}
            },
            "password": passwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        # 调用oss接口,接口失败重试3次，每次间隔3s
        retry_nums = 0
        while retry_nums < 3:
            retry_nums += 1
            if retry_nums != 1:
                time.sleep(3)
            ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
            log.info(f'query_db_version ret_body : {ret_body}, retry_nums:  {retry_nums}')
            if not ret:
                log.error(f'Failed query_db_version, ret_body is : {ret_body}')
                continue
            if ret_body.get("returnMsg") != "ok":
                log.error(f'query_db_version error with return: {ret_body.get("returnMsg")}')
                continue
            return ret_body.get("returnData")
        return []

    def shard_binlog(self, request_url, group_id, env_variable):
        user, passwd = self.get_user_passwd(env_variable)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.ShardBinlog",
                "para": {
                    "groupid": group_id
                }
            },
            "password": passwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        # 调用oss接口,接口失败重试3次，每次间隔3s
        retry_nums = 0
        while retry_nums < 3:
            retry_nums += 1
            if retry_nums != 1:
                time.sleep(3)
            ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
            log.info(f'shard_binlog ret_body : {ret_body},, retry_nums:  {retry_nums}')
            if not ret:
                log.error(f'Failed shard_binlog, ret_body is : {ret_body}')
                continue
            if ret_body.get("returnMsg") != "ok":
                log.error(f'shard_binlog error with return: {ret_body.get("returnMsg")}')
                continue
            return_code = ret_body.get("returnData").get("code")
            log.info(f'shard_binlog return_code : {return_code}')
            return return_code == 0
        return False

    def create_tdsql_instance(self, request_url, env_variable, instance_info):
        user, passwd = self.get_user_passwd(env_variable)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL",
            "caller": user,
            "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.AddGWInstance",
                "para": instance_info
            },
            "password": passwd,
            "timestamp": timestamp,
            "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        # 调用oss接口,接口失败重试3次，每次间隔3s
        retry_nums = 0
        while retry_nums < 3:
            retry_nums += 1
            ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
            log.info(f'create_tdsql_instance ret_body : {ret_body}, retry_nums:  {retry_nums}')
            if not ret or ret_body.get("returnMsg") != "ok":
                log.error(f'create_tdsql_instance error with return: {ret_body.get("returnMsg")}')
                time.sleep(3)
                continue
            return True, ret_body.get("returnData")
        return False, []

    def query_instance_progress(self, request_url, env_variable, task_id):
        user, passwd = self.get_user_passwd(env_variable)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL",
            "caller": user,
            "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.QueryAddGWInsance",
                "para": task_id
            },
            "password": passwd,
            "timestamp": timestamp,
            "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        # 调用oss接口,接口失败重试3次
        retry_nums = 0
        while retry_nums < 3:
            retry_nums += 1
            if retry_nums != 1:
                time.sleep(3)
            ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
            log.info(f'query_instance_progress ret_body : {ret_body}, retry_nums:  {retry_nums}')
            if not ret:
                log.error(f'Failed query_instance_progress, ret_body is : {ret_body}, retry_nums is {retry_nums}')
                continue
            if ret_body.get("returnMsg") != "ok":
                log.error(f'query_instance_progress error with return: {ret_body.get("returnMsg")}')
                continue
            ret_data = ret_body.get("returnData")
            status = ret_data.get("status")
            return status, ret_data
        return -1, ""

    def init_tdsql_instance(self, request_url, env_variable, instance_id):
        user, passwd = self.get_user_passwd(env_variable)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL",
            "caller": user,
            "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.InitInstance",
                "para": {"id": instance_id}
            },
            "password": passwd,
            "timestamp": timestamp,
            "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        # 调用oss接口,接口失败重试3次，每次间隔3s
        retry_nums = 0
        while retry_nums < 3:
            retry_nums += 1
            if retry_nums != 1:
                time.sleep(3)
            ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
            log.info(f'init_tdsql_instance ret_body : {ret_body}, retry_nums:  {retry_nums}')
            if not ret:
                log.error(f'Failed init_tdsql_instance, ret_body is : {ret_body}')
                continue
            if ret_body.get("returnMsg") != "ok":
                log.error(f'init_tdsql_instance error with return: {ret_body.get("returnMsg")}')
                continue
            return True, ret_body.get("returnData")
        return False, []

    def query_init_instance(self, request_url, env_variable, taskid):
        user, passwd = self.get_user_passwd(env_variable)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL",
            "caller": user,
            "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.QueryInitInstance",
                "para": taskid
            },
            "password": passwd,
            "timestamp": timestamp,
            "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        # 调用oss接口,接口失败重试3次，每次间隔3s
        retry_nums = 0
        while retry_nums < 3:
            retry_nums += 1
            if retry_nums != 1:
                time.sleep(3)
            ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
            log.info(f'query_init_instance ret_body : {ret_body}, retry_nums:  {retry_nums}')
            if not ret:
                log.error(f'Failed query_init_instance, ret_body is : {ret_body}')
                continue
            if ret_body.get("returnMsg") != "ok":
                log.error(f'query_init_instance error with return: {ret_body.get("returnMsg")}')
                continue
            return True, ret_body.get("returnData")
        return False, []

    def query_coldbackup_node(self, env_variable, request_url, group_id, set_id, pid):
        log.info(f"start query_coldbackup_node.")
        user, passwd = self.get_user_passwd(env_variable)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL",
            "caller": user,
            "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.GetColdbackupElectZkNode",
                "para": {
                    "groupid": group_id,
                    "id": set_id
                }
            },
            "password": passwd,
            "timestamp": timestamp,
            "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        # 调用oss接口,接口失败重试3次，每次间隔3s
        retry_nums = 0
        while retry_nums < 3:
            retry_nums += 1
            if retry_nums != 1:
                time.sleep(3)
            ret, ret_body, ret_header = request_post(request_url, request_body, request_header)
            log.info(f'query_coldbackup_node ret_body : {ret_body}, retry_nums:  {retry_nums}')
            if not ret:
                log.error(f'Failed query_coldbackup_node, ret_body is : {ret_body}')
                continue
            if ret_body.get("returnMsg") != "ok":
                log.error(f'query_coldbackup_node error with return: {ret_body.get("returnMsg")}')
                continue
            return True, ret_body.get("returnData")
        return False, {}
