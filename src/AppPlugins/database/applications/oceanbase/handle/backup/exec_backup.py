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
import json
import os
import re
import shutil
import stat
import threading
import time
import datetime

import pymysql

from common.cleaner import clear
from common.common import output_execution_result_ex, output_result_file, execute_cmd, invoke_rpc_tool_interface, \
    execute_cmd_list, touch_file, ismount_with_timeout, read_tmp_json_file
from common.common_models import SubJobModel, SubJobDetails, CopyInfoRepModel, Copy, ReportCopyInfoModel, LogDetail, \
    RepositoryPath, ScanRepositories
from common.const import ParamConstant, SubJobPolicyEnum, SubJobPriorityEnum, BackupTypeEnum, ExecuteResultEnum, \
    RepositoryDataTypeEnum, SubJobStatusEnum, RepositoryNameEnum
from common.util.exec_utils import su_exec_rm_cmd, exec_mkdir_cmd

from oceanbase.common.const import OceanBaseSubJobName, SubJobType, OceanBaseCode, ActionResponse, ErrorCode, \
    OceanBaseBackupLevel, OceanBaseQueryStatus, OceanBaseSqlCmd, copyMetaFileName, CMDResult, RpcParamKey, \
    LastCopyType, SubJobStatusForSqlite, ObclientStatus, OceanBaseReportLabel, LogLevel
from oceanbase.common.oceanbase_backup_exception import LogDetailException
from oceanbase.common.oceanbase_common import remove_dir, get_env_variable, exec_rc_tool_cmd, report_job_details, \
    get_dir_size, init_sqlite_file, wait_or_lock_sqlite, update_sqlite_sub_job_status, get_agent_id, \
    check_special_characters, str_to_float, check_mount
from oceanbase.common.oceanbase_exception import ErrCodeException
from oceanbase.common.oceanbase_sqlite_service import OceanBaseSqliteService
from oceanbase.logger import log


class BackUp:

    def __init__(self, pid, job_id, sub_job_id, data, json_param):
        if not json_param:
            log.error("Parse params obj is null.")
            raise Exception("Parse params obj is null.")
        self._json_param = json_param
        self._pid = pid
        self._job_id = job_id
        self._sub_job_id = sub_job_id
        self._std_in = data

        self._protect_env = self._json_param.get("job", {}).get("protectEnv", {})
        self._protect_object = self._json_param.get("job", {}).get("protectObject", {})
        self._repositories = self._json_param.get("job", {}).get("repositories", [])
        self._cache_area = self.get_repository_path(json_param, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        self._data_area = self.get_repository_path(json_param, RepositoryDataTypeEnum.DATA_REPOSITORY)
        self._meta_area = self.get_repository_path(json_param, RepositoryDataTypeEnum.META_REPOSITORY)
        self._log_area = self.get_repository_path(json_param, RepositoryDataTypeEnum.LOG_REPOSITORY)

        self._backup_type = self._json_param.get("job", {}).get("jobParam", {}).get("backupType", "")

        self._copy_id = self._json_param.get("job", {}).get("copy", [])[0].get("id", "")

        self._query_time_interval = 3
        self._cluster_name, self._cluster_id = self.get_cluster_name_and_id(self._protect_env)
        if not check_special_characters(self._cluster_name):
            log.error(f'the cluster_name is Illegal {self._cluster_name}')
            raise Exception(f'the cluster_name is Illegal {self._cluster_name}')
        self._persistent_mount = os.path.join("/", self._cluster_name, self._cluster_id)
        self._tenant_name_list = []  # tenant_name不带sys租户
        self._tenant_id_list = []  # tenant_id在获取后，将1加到第一个元素
        self._tenant_name_id_list = []  # tenant_name_id不带sys租户与1
        self._bs_key = 0
        self._incarnation_id = 0
        self._backup_time = ''
        self._max_next_time = ''

        self._job_status = SubJobStatusEnum.RUNNING

    @staticmethod
    def get_enter_database_ip_port(protect_env):
        cluster_info_str = protect_env.get("extendInfo", {}).get("clusterInfo", "")
        cluster_info = json.loads(cluster_info_str)
        observer_list = cluster_info.get("obServerAgents", [])
        observer_info_dict = observer_list[0]
        ip = observer_info_dict.get("ip", "")
        port = int(observer_info_dict.get("port", ""))
        return ip, port

    @staticmethod
    def get_obclient_and_observer_node_list(protect_env):
        cluster_info_str = protect_env.get("extendInfo", {}).get("clusterInfo", "")
        cluster_info = json.loads(cluster_info_str)
        obclient_list = cluster_info.get("obClientAgents", [])
        obclient_uuid_list = []
        for node in obclient_list:
            if node.get("linkStatus", "") == ObclientStatus.ONLINE.value:
                uuid = node.get("parentUuid", "")
                obclient_uuid_list.append(uuid)
        observer_list = cluster_info.get("obServerAgents", [])
        observer_uuid_list = []
        for node in observer_list:
            if node.get("linkStatus", "") == ObclientStatus.ONLINE.value:
                uuid = node.get("parentUuid", "")
                observer_uuid_list.append(uuid)
        nodes = protect_env.get("nodes", [])
        obclient_node_list = []
        observer_node_list = []
        for uuid in obclient_uuid_list:
            for node in nodes:
                if uuid == node.get("id", ""):
                    obclient_node_list.append(node)
                    break
        for uuid in observer_uuid_list:
            for node in nodes:
                if uuid == node.get("id", ""):
                    observer_node_list.append(node)
                    break
        return obclient_node_list, observer_node_list

    @staticmethod
    def allow_backup_in_local_node(req_id, job_id, json_param):
        agent_infos = json_param.get("job", {}).get('extendInfo', {}).get('agents', [])
        agent_uuids = set()
        for agent_info in agent_infos:
            agent_uuids.add(agent_info['id'])
        agent_uuid_list = list(agent_uuids)
        protect_env = json_param.get("job", {}).get("protectEnv", {})
        nodes = protect_env.get("nodes", [])
        for node in nodes:
            uuid = node.get("id", "")
            if uuid not in agent_uuid_list:
                response = ActionResponse(code=ExecuteResultEnum.INTERNAL_ERROR,
                                          bodyErr=ErrorCode.ERR_ENVIRONMENT,
                                          message=f"{uuid} is offline")
                output_result_file(req_id, response.dict(by_alias=True))
                log.info(f"agent({uuid}) is offline and not in agent list")
                return
        log.info("agent list check successfully")
        response = ActionResponse(code=ExecuteResultEnum.SUCCESS)
        output_result_file(req_id, response.dict(by_alias=True))

    def query_scan_repositories(self):
        # E6000适配
        log.info(f"Query scan repositories, job_id: {self._job_id}.")
        if self._backup_type == BackupTypeEnum.LOG_BACKUP.value:
            # log仓的meta区 /Database_{resource_id}_LogRepository_su{num}/{ip}/meta/{job_id}
            meta_copy_path = os.path.join(os.path.dirname(self._log_area), RepositoryNameEnum.META, self._job_id)
            # log仓的data区 /Database_{resource_id}_LogRepository_su{num}/{ip}/{job_id}
            data_path = self._log_area
            # /Database_{resource_id}_LogRepository_su{num}/{ip}
            save_path = os.path.dirname(self._log_area)
        else:
            # meta/Database_{resource_id}_InnerDirectory_su{num}/source_policy_{job_id}/Context_Global_MD/{ip}
            meta_copy_path = self._meta_area
            # data/Database_{resource_id}_InnerDirectory_su{num}/source_policy_{job_id}/Context/{ip}
            data_path = self._data_area
            # meta/Database_{resource_id}_InnerDirectory_su{num}/source_policy_{job_id}/Context_Global_MD/{ip}
            save_path = self._meta_area
        if not os.path.exists(meta_copy_path):
            exec_mkdir_cmd(meta_copy_path, mode=0x777)
        log_meta_copy_repo = RepositoryPath(repositoryType=RepositoryDataTypeEnum.META_REPOSITORY.value,
                                            scanPath=meta_copy_path)
        log_data_repo = RepositoryPath(repositoryType=RepositoryDataTypeEnum.LOG_REPOSITORY.value,
                                       scanPath=data_path)
        scan_repos = ScanRepositories(scanRepoList=[log_data_repo, log_meta_copy_repo], savePath=save_path)
        output_result_file(self._pid, scan_repos.dict(by_alias=True))
        log.info(f"Query scan repos success, return result {scan_repos}, job id: {self._job_id}")

    @staticmethod
    def get_tenant_name_list(protect_object):
        cluster_info_str = protect_object.get("extendInfo", {}).get("clusterInfo", "")
        cluster_info = json.loads(cluster_info_str)
        tenant_info = cluster_info.get('tenantInfos', [])
        tenant_name_list = []
        for tenant in tenant_info:
            tenant_name = tenant.get("name", "")
            tenant_name_list.append(tenant_name)
        log.info(f'tenant_name_list is : {tenant_name_list}')
        return tenant_name_list

    @staticmethod
    def get_cluster_name_and_id(protect_object):
        cluster_info_str = protect_object.get("extendInfo", {}).get("clusterInfo", "")
        cluster_info = json.loads(cluster_info_str)
        cluster_id = cluster_info.get("cluster_id", "")
        cluster_name = cluster_info.get("cluster_name", "")
        return cluster_name, cluster_id

    @staticmethod
    def get_repository_path(json_param, repository_type):
        repositories = json_param.get("job", {}).get("repositories", [])
        repositories_path = ""
        for repository in repositories:
            if repository['repositoryType'] == repository_type:
                repositories_path = repository["path"][0]
                break
        return repositories_path

    @staticmethod
    def set_error_response(response):
        response.code = OceanBaseCode.FAILED.value
        response.body_err = OceanBaseCode.FAILED.value

    @staticmethod
    def backup_sql_cmd(cmd, tenant_id_list=None, bs_key=0, tenant_name_list="", backup_destination="", last_bs_key=0):
        if tenant_id_list is None:
            tenant_id_list = ["NONE", "NONE"]
        sql_dict = {
            OceanBaseSqlCmd.MAX_BS_KEY_STATUS: f'SELECT STATUS FROM oceanbase.CDB_OB_BACKUP_SET_FILES \
                                                WHERE TENANT_ID IN ({str(tenant_id_list)[1:-1]}) AND BS_KEY={bs_key}',
            OceanBaseSqlCmd.QUERY_LOG_STATUS: f'SELECT STATUS FROM oceanbase.CDB_OB_BACKUP_ARCHIVELOG',
            OceanBaseSqlCmd.CLUSTER_FULL_BACKUP: 'ALTER SYSTEM BACKUP DATABASE',
            OceanBaseSqlCmd.CLUSTER_INCRE_BACKUP: 'ALTER SYSTEM BACKUP INCREMENTAL DATABASE',
            OceanBaseSqlCmd.TENANT_FULL_BACKUP: f'ALTER SYSTEM BACKUP TENANT {tenant_name_list} \
                                                TO "{backup_destination}"',
            OceanBaseSqlCmd.QUERY_BACKUP_DESTINATION: "SELECT VALUE FROM oceanbase. __all_sys_parameter \
                                                WHERE name='backup_dest'",
            OceanBaseSqlCmd.QUERY_INCARNATION_ID: f"SELECT INCARNATION FROM oceanbase.CDB_OB_BACKUP_ARCHIVELOG",
            OceanBaseSqlCmd.QUERY_LOG_MAX_NEXT_TIME: "SELECT TENANT_ID,MAX_NEXT_TIME FROM \
                                                oceanbase.CDB_OB_BACKUP_ARCHIVELOG",
            OceanBaseSqlCmd.QUERY_TENANT_ID_LIST: "SELECT tenant_id FROM oceanbase.gv$tenant",
            OceanBaseSqlCmd.QUERY_BACKUP_TIME: f'SELECT MAX(COMPLETION_TIME) FROM oceanbase.CDB_OB_BACKUP_SET_FILES \
                                                WHERE TENANT_ID IN ({str(tenant_id_list)[1:-1]}) AND BS_KEY={bs_key}',
            OceanBaseSqlCmd.QUERY_TENANT_ID_LIST_BY_NAME: f'SELECT tenant_name, tenant_id FROM oceanbase.gv$tenant \
                                                WHERE tenant_name IN ({str(tenant_name_list)[1:-1]})',
            OceanBaseSqlCmd.QUERY_TENANT_NAME_LIST_BY_ID: f'SELECT tenant_name, tenant_id FROM oceanbase.gv$tenant \
                                            WHERE tenant_id IN ({str(list(tenant_id_list[1:]))[1:-1]})',
            OceanBaseSqlCmd.UPDATE_LOG_ARCHIVE_INTERVAL:
                "alter system set backup_dest_option='log_archive_checkpoint_interval=30s'",
            OceanBaseSqlCmd.QUERY_DATABASE_FOR_DISPLAY: f'select distinct tenant_name, database_name from '
                                                        f'oceanbase.gv$table where table_type =3 and '
                                                        f'tenant_id in ({str(list(tenant_id_list[1:]))[1:-1]});',
            OceanBaseSqlCmd.QUERY_TABLE_FOR_DISPLAY: f'select tenant_name, database_name, table_name from '
                                                     f'oceanbase.gv$table where  table_type =3 and '
                                                     f'tenant_id in ({str(list(tenant_id_list[1:]))[1:-1]}) ;',
            OceanBaseSqlCmd.QUERY_OLD_BACKUP_SET: f"select distinct bs_key from oceanbase.CDB_OB_BACKUP_SET_DETAILS "
                                                  f"where BACKUP_LEVEL='CLUSTER' and STATUS='COMPLETED' "
                                                  f"and BS_KEY<{bs_key} and BS_KEY>={last_bs_key}"
        }
        sql_str = sql_dict.get(cmd)
        log.info(f'sql_str : {sql_str}')
        return sql_str

    @staticmethod
    def exec_oceanbase_sql(ip, port, sql_str, pid):
        conn = BackUp.get_db_session(ip, port, pid)
        try:
            cur = conn.cursor()
            cur.execute(sql_str)
        except Exception as ex:
            log.error(f"execute sql {sql_str} failed!, ex is {ex}")
            return False, ex
        return True, cur.fetchall()

    @staticmethod
    def get_db_session(ip, port, pid):
        user = get_env_variable(f'job_protectEnv_auth_authKey_{pid}')
        db_pwd = get_env_variable(f'job_protectEnv_auth_authPwd_{pid}')
        try:
            return pymysql.connect(host=ip, port=port, user=user, passwd=db_pwd)
        except Exception as except_str:
            log.error(f"Connect MySQL :{ip} service failed!")
            raise ErrCodeException(ErrorCode.ERROR_AUTH, "Check connectivity: auth info error!") from except_str
        finally:
            clear(db_pwd)

    @staticmethod
    def exec_oceanbase_sql_with_expect(ip, port, sql_str, expect_output, pid):
        log.info(f"exec_oceanbase_sql_with_expect starts, sql_str is {sql_str}, expect_output is {expect_output}")
        output = ''
        try:
            conn = BackUp.get_db_session(ip, port, pid)
            cur = conn.cursor()
        except Exception as err:
            log.error(f"execute sql {sql_str} failed!")
            return False, err
        while not BackUp.check_sql_output(output, expect_output):
            try:
                cur.execute(sql_str)
                output = cur.fetchall()
            except Exception as err:
                log.error(f"execute sql {sql_str} failed!")
                cur.close()
                conn.close()
                return False, err
            if OceanBaseQueryStatus.FAILED.value in str(output):
                log.error(f"execute sql {sql_str} failed, output STATUS is {output} ")
                cur.close()
                conn.close()
                return False, Exception("Status FAILED in output")
            log.info(f"now the output is {output} and expect output is {expect_output}")
            time.sleep(10)
        cur.close()
        conn.close()
        return True, output

    @staticmethod
    def check_sql_output(output, expect_output):
        if len(set(output)) == 1 and output[0][0] == expect_output:
            return True
        else:
            return False

    @staticmethod
    def exec_multi_process_cmd(cmd_list):
        threads = []
        for cmd in cmd_list:
            copy_thread = threading.Thread(target=execute_cmd, args=(cmd,))  # 调用函数,引入线程参数
            copy_thread.start()  # 开始执行
            threads.append(copy_thread)
        for copy_thread in threads:
            copy_thread.join()

    @staticmethod
    def check_backup_job_level(protect_object):
        sub_type = protect_object.get("subType", "")
        if sub_type == "OceanBase-cluster":
            return OceanBaseBackupLevel.BACKUP_CLUSTER_LEVEL
        else:
            return OceanBaseBackupLevel.BACKUP_TENANT_LEVEL

    @staticmethod
    def remove_tmp_file(file_path):
        if os.path.exists(file_path):
            remove_dir(file_path)
        return True

    @staticmethod
    def calculate_speed(speed_file, copy_path):
        if not os.path.exists(speed_file):
            log.info("speed file has been removed, return False")
            return False, 0
        log.info("RUNNING and calculating")
        with open(speed_file, "r", encoding='UTF-8') as f_content:
            content = f_content.read()
            speed_info = json.loads(content)

        data_size_old = speed_info.get('data_size', int)
        time_old = speed_info.get('time', int)

        data_size_new = get_dir_size(copy_path)
        time_new = int((time.time()))

        log.info(f"time_new and time_old is {time_new, time_old}")
        log.info(f"data_size_new and data_size_old is {data_size_new, data_size_old}")
        data_size_diff = int(data_size_new) - int(data_size_old)
        time_diff = time_new - int(time_old)
        speed_info = {
            'data_size': data_size_new,
            'time': time_new
        }
        output_execution_result_ex(speed_file, speed_info)
        if not time_diff:
            log.info(f"query_size_and_speed, time_diff is {time_diff}")
            return 0, data_size_diff
        else:
            try:
                speed = data_size_diff / time_diff
                return speed, data_size_diff
            except Exception:
                log.error("Error while calculating speed!")
                return 0, 0

    @staticmethod
    def copy_log_incremental(tenant_id_list, clog_source, copy_to_path, period):
        for tenant_id in tenant_id_list:
            copy_from_path_tenant_id = os.path.join(clog_source, f'{tenant_id}')
            copy_to_path_tenant_id = os.path.join(copy_to_path, f'{tenant_id}')
            os.chdir(copy_from_path_tenant_id)
            cp_cmd_list = [
                f"find ./clog/ -mmin -{period} -type f",
                "xargs -i cp --parents -r {} " + copy_to_path_tenant_id
            ]
            return_code, out_info, err_info = execute_cmd_list(cp_cmd_list)
            if return_code != CMDResult.SUCCESS:
                log.error(f'execute copy cmd failed, message: {out_info}, err: {err_info}')
                return False
        return True

    @staticmethod
    def build_period_for_log_copy(last_backup_time, max_next_time):
        if last_backup_time > max_next_time:
            log.error(f'{max_next_time} is earlier than last_backup_time: {last_backup_time}.')
            raise Exception("max_next_time is earlier than last_backup_time")
        # 多拷贝10min日志，避免数据不完整
        time_diff_min = (max_next_time - last_backup_time) / 60 + 10
        log.info(f"last_backup_time is {last_backup_time}, max_next_time is {max_next_time}")
        return time_diff_min

    @staticmethod
    def report_error_label_and_continue(pid, error_code: ErrorCode):
        response = ActionResponse(code=ExecuteResultEnum.CONTINUE,
                                  bodyErr=error_code,
                                  message="")
        output_result_file(pid, response.dict(by_alias=True))

    @staticmethod
    def get_local_ip(json_param):
        current_agent_id = get_agent_id()
        local_ip = None
        nodes = json_param['job']['protectEnv']['nodes']
        for node in nodes:
            if current_agent_id == node['id']:
                local_ip = node['endpoint']
                break
        return local_ip

    @staticmethod
    def get_last_backup_time_and_id(last_copy_info):
        copy_type = last_copy_info.get("type", "")
        log.info(f"get_last_any_copy_type copy_type is {copy_type}")
        if not copy_type:
            return {}, {}
        extend_info = last_copy_info.get("extendInfo", {})
        if copy_type == RpcParamKey.LOG_COPY:
            timestamp = extend_info.get("endTime", 0)
            last_copy_id = extend_info.get("associatedCopies", [])
        else:
            timestamp = extend_info.get("backup_time", 0)
            last_copy_id = extend_info.get("copy_id", "")
        log.info(f'timestamp, last_copy_id info:{timestamp, last_copy_id}')
        return timestamp, last_copy_id

    @staticmethod
    def exe_mount(data_remote_path, data_repo, mount_point):
        remote_hosts = data_repo.get("remoteHost")
        for remote_host in remote_hosts:
            remote_ip = remote_host.get("ip")
            mount_cmd_str = f"sudo mount -tnfs4 -o rw,nfsvers=4.1,sync,lookupcache=positive,hard,timeo=600," \
                            f"wsize=1048576,rsize=1048576,namlen=255 {remote_ip}:{data_remote_path} {mount_point}"
            return_code, out_info, err_info = execute_cmd(mount_cmd_str)
            if return_code == CMDResult.SUCCESS:
                break
            else:
                log.error(f"The execute mount cmd failed! ERROR_INFO : {err_info}")
        return return_code

    def gen_sub_job(self):
        node_obclient_list, node_observer_list = self.get_obclient_and_observer_node_list(self._protect_env)
        if not node_obclient_list or not node_observer_list:
            log.error(f"obclient or observer agent list is empty, gen sub job failed ")
            return False
        log.info(f'gen_sub_job backup_type is {self._backup_type}')
        if self._backup_type == BackupTypeEnum.LOG_BACKUP:
            self.gen_sub_job_log(node_obclient_list, node_observer_list)
        else:
            self.gen_sub_job_data(node_obclient_list, node_observer_list)

        sqlite_file_name = os.path.join(self._meta_area, 'sqlite_file', f'{self._job_id}')
        sqlite_file_path = os.path.join(self._meta_area, "sqlite_file")
        if not os.path.exists(sqlite_file_path):
            os.makedirs(sqlite_file_path)
        if not init_sqlite_file(sqlite_file_name, len(node_obclient_list)):
            return False
        return True

    def gen_sub_job_data(self, node_obclient_list, node_observer_list):
        log.info("step 2-4 : start to gen_sub_job for data backup")
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        sub_job_list_data = []
        # priority 1: mount repository;execute on all nodes(obclient + observer)
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_1
        job_policy = SubJobPolicyEnum.FIXED_NODE.value
        job_name = OceanBaseSubJobName.SUB_EXEC_MOUNT_JOB
        for node in node_observer_list:
            sub_job = self.build_sub_job(job_priority, job_policy, job_name, node)
            sub_job_list_data.append(sub_job)

        # priority 2: check log status;execute on one observer node only
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_2
        job_policy = SubJobPolicyEnum.FIXED_NODE.value
        job_name = OceanBaseSubJobName.SUB_CHECK_LOG_STATUS
        node = node_observer_list[0]
        sub_job = self.build_sub_job(job_priority, job_policy, job_name, node)
        sub_job_list_data.append(sub_job)

        # priority 3: exec data backup;execute on one observer node only
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_3
        job_policy = SubJobPolicyEnum.FIXED_NODE.value
        job_name = OceanBaseSubJobName.SUB_EXEC_DATA_BACKUP
        node = node_observer_list[0]
        sub_job = self.build_sub_job(job_priority, job_policy, job_name, node)
        sub_job_list_data.append(sub_job)

        log.info(f"gen_sub_job_data get sub_job_array: {sub_job_list_data}")
        output_execution_result_ex(file_path, sub_job_list_data)  # 结果文件清空再写
        log.info(f"step 2-4: backup data gen_sub_job succeeded.sub_job amount:{len(sub_job_list_data)}")

    def gen_sub_job_log(self, node_obclient_list, node_observer_list):
        log.info("step 2-4 : start to gen_sub_job for log backup")
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        sub_job_list_log = []

        # priority 1: check log status;execute on one observer node only
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_1
        job_policy = SubJobPolicyEnum.FIXED_NODE.value
        job_name = OceanBaseSubJobName.SUB_CHECK_LOG_STATUS
        node = node_observer_list[0]
        sub_job = self.build_sub_job(job_priority, job_policy, job_name, node)
        sub_job_list_log.append(sub_job)

        # priority 2: exec log backup;execute on one observer node only
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_2
        job_policy = SubJobPolicyEnum.FIXED_NODE.value
        job_name = OceanBaseSubJobName.SUB_EXEC_LOG_COPY
        node = node_observer_list[0]
        sub_job = self.build_sub_job(job_priority, job_policy, job_name, node)
        sub_job_list_log.append(sub_job)

        log.info(f"gen_sub_job_log get sub_job_array: {sub_job_list_log}")
        output_execution_result_ex(file_path, sub_job_list_log)  # 结果文件清空再写
        log.info(f"step 2-4: backup log gen_sub_job_log succeeded.sub_job amount:{len(sub_job_list_log)}")

    def build_sub_job(self, job_priority, job_policy, job_name, node):
        node_id = node.get("id", "")
        endpoint = node.get("endpoint", "")
        port = str(node.get("port", ""))
        host = endpoint + ":" + port
        job_info = f"{host}"
        return SubJobModel(jobId=self._job_id, jobType=SubJobType.BUSINESS_SUB_JOB.value, execNodeId=node_id,
                           jobPriority=job_priority, jobName=job_name, policy=job_policy, jobInfo=job_info,
                           ignoreFailed=False).dict(by_alias=True)

    def exec_mount_job(self):
        log.info(f"exec_mount_job {self._job_id}")
        mount_point = os.path.join("/", self._cluster_name, self._cluster_id)
        data_repo = None
        for repository in self._repositories:
            if repository.get("repositoryType", "") == RepositoryDataTypeEnum.DATA_REPOSITORY.value:
                data_repo = repository
        data_remote_path = data_repo.get("remotePath", "")
        # 如果挂载点已经挂载data仓的remotePath：直接返回成功
        if check_mount(mount_point) and self.deal_mount(mount_point):
            return True
        else:
            # 如果是租户集备份，挂载不在了，直接报错
            if self.check_backup_job_level(self._protect_object) == OceanBaseBackupLevel.BACKUP_TENANT_LEVEL:
                err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.FULL_BACK_SHOULD_BEFORE_TENANT_SET_BACKUP_LABEL,
                                           logLevel=LogLevel.ERROR)
                raise LogDetailException(log_detail=err_log_detail)
        # 判断挂载点状态，检测是否被服务端解挂载
        return_code, out_info, err_info = execute_cmd(f'cd {mount_point}')
        if return_code != CMDResult.SUCCESS:
            # 执行一次解挂载,并且记录状态,用于重启开启ob备份
            execute_cmd(f'umount -l {mount_point}')
            log.info("backup dest was unmounted from X8000, execute umount command here")
            reopen_log_file = os.path.join(self._meta_area, f"reopen_log_{self._job_id}")
            touch_file(reopen_log_file)
            log.info(f"create reopen_log_file here")
        local_ip = self.get_local_ip(self._json_param)
        log.warning(f"mount point has not been mounted yet")
        # 没有挂载：创建挂载点 mkdir -p /{cluster_name}/{cluster_id}, 并chown
        return_code, out_info, err_info = execute_cmd(f'mkdir -p {mount_point}')
        log.info(f"mkdir return_code, out_info, err_info is {return_code, out_info, err_info}")
        if return_code != CMDResult.SUCCESS or not os.path.exists(mount_point):
            log.error(f"The execute mkdir_local_dir_path cmd failed! ERROR_INFO : {err_info}")
            err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_MKDIR_MOUNT_POINT_FAIL_LABEL,
                                       logInfoParam=[local_ip], logLevel=LogLevel.ERROR)
            raise LogDetailException(log_detail=err_log_detail)
        # 执行挂载命令
        return_code = self.exe_mount(data_remote_path, data_repo, mount_point)
        if return_code != CMDResult.SUCCESS:
            err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_MOUNT_FAIL_LABEL, logInfoParam=[local_ip],
                                       logLevel=LogLevel.ERROR)
            raise LogDetailException(log_detail=err_log_detail)
        # 修改权限，因为里面有的目录不能修改，所以肯定会报错，但不影响任务执行
        execute_cmd(f'chmod -R 750 {os.path.join("/", self._cluster_name)}')
        execute_cmd(f'chown -R admin:admin {os.path.join("/", self._cluster_name)}')
        # 将挂载信息写入/etc/fstab "x8000文件系统 挂载点 nfs defaults 1 1"
        # 等待10s，否则检查挂载可能失败
        time.sleep(10)
        # 重新检查是否挂载成功(因为是刚挂载的，应该不存在网不通挂载路径还在的情况，就不会卡死)
        if not os.path.ismount(mount_point):
            log.error("fail to mount x8000 to observer")
            err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_MOUNT_FAIL_LABEL, logInfoParam=[local_ip],
                                       logLevel=LogLevel.ERROR)
            raise LogDetailException(log_detail=err_log_detail)
        return True

    def deal_mount(self, mount_point):
        log.info(f"mount point has already been mounted")
        check_cmd = ["mount", f"grep {mount_point}"]
        return_code, std_out, std_err = execute_cmd_list(check_cmd)
        if return_code != CMDResult.SUCCESS:
            log.error(f"Fail to get mount info {std_err}")
            return True
        if self._protect_env.get("id", "") in std_out:
            return True
        # 解挂载
        ret, _, err = execute_cmd(f"umount -l {mount_point}")
        if ret != CMDResult.SUCCESS:
            log.error(f"Fail to umount {err}")
            return True
        try:
            shutil.rmtree(os.path.join("/", self._cluster_name))
        except Exception as err:
            log.error(f"Fail to remove mount path for resource, {err}")
        return False

    def pre_check_before_sub_job(self, priority):
        sqlite_file_name = os.path.join(self._meta_area, 'sqlite_file', f'{self._job_id}')
        ret = wait_or_lock_sqlite(sqlite_file_name=sqlite_file_name,
                                  timeout=20,
                                  sub_job_priority=priority.value)
        while ret == SubJobStatusForSqlite.DOING.value:
            time.sleep(10)
            ret = wait_or_lock_sqlite(sqlite_file_name=sqlite_file_name,
                                      timeout=20,
                                      sub_job_priority=priority.value)
            log.info("waiting other node execute this sub job")
        if ret == SubJobStatusForSqlite.SUCCESS.value:
            return True
        elif ret == SubJobStatusForSqlite.FAILED.value:
            return False
        conn = ret
        return conn

    def exec_check_log_status_sub_job(self):
        log.info("sub_job_1: exec_check_log_status_sub_job starts")
        # 检查obclient是否可以成功连接observer
        ret = self.pre_check_before_sub_job(SubJobPriorityEnum.JOB_PRIORITY_1)
        if isinstance(ret, bool):
            return ret
        conn = ret
        log.info("local node get conn lock")
        ip, port = self.get_enter_database_ip_port(self._protect_env)
        # 检查连通性，如果obclient连接observer失败：1还有其他节点没执行：上报任务成功，转移其他节点执行；2全部失败：上报任务失败
        if not self.check_obclient_connection(ip, port, self._pid):
            if not update_sqlite_sub_job_status(conn=conn, updated_status=SubJobStatusForSqlite.RETRY.value,
                                                priority=SubJobPriorityEnum.JOB_PRIORITY_1.value):
                err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_ALL_OBCLIENT_OFFLINE_LABEL,
                                           logInfoParam=[], logLevel=LogLevel.ERROR)
                raise LogDetailException(log_detail=err_log_detail)
            log.debug("local node fail to connect observer, update sqlite status to RETRY")
            self.report_error_label_and_continue(self._pid, ErrorCode.ERROR_INTERNAL)
            # ErrorCode.WARN_CONN_FAILED_TRANSFER_OTHER_NODES
            return True
        # 备份前检查数据库是否除了sys租户，没有其他业务租户
        self.check_tenant_is_null(ip, port, conn)
        # 校验是否需要手动开启备份
        self.backup_dest_check_if_need(ip, port, conn)
        # 检查archive log是否已成功开启
        if not self.check_log_status(ip, port):
            log.error("oceanbase status is not DOING")
            update_sqlite_sub_job_status(conn=conn, updated_status=SubJobStatusForSqlite.FAILED.value,
                                         priority=SubJobPriorityEnum.JOB_PRIORITY_1.value)
            err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_OPEN_LOG_ARCHIVE_FAIL_LABEL,
                                       logInfoParam=[], logLevel=LogLevel.ERROR)
            raise LogDetailException(log_detail=err_log_detail)
        # 更新backup_dest_option，暂定失败了不影响结果
        self.update_log_archive_interval(ip, port)
        # 更新sqlite为已经成功执行该子任务，其他正在等待的节点等待结束，不再需要执行任务，直接返回成功
        update_sqlite_sub_job_status(conn=conn, updated_status=SubJobStatusForSqlite.SUCCESS.value,
                                     priority=SubJobPriorityEnum.JOB_PRIORITY_1.value)
        log.info("sub_job_1: exec_check_log_status_sub_job succeeded")
        return True

    def check_tenant_is_null(self, ip, port, conn):
        sql_str = self.backup_sql_cmd(OceanBaseSqlCmd.QUERY_TENANT_ID_LIST)
        ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_EMPTY_CLUSTER_FAIL_LABEL,
                                   logInfoParam=[], logLevel=LogLevel.ERROR)
        if not ret:
            update_sqlite_sub_job_status(conn=conn, updated_status=SubJobStatusForSqlite.FAILED.value,
                                         priority=SubJobPriorityEnum.JOB_PRIORITY_2.value)
            raise LogDetailException(log_detail=err_log_detail)
        # 查询的租户id如果只有sys租户，任务失败，抛出相应的error label
        if len(output) == 1:
            log.error("sys tenant only, backup failed")
            update_sqlite_sub_job_status(conn=conn, updated_status=SubJobStatusForSqlite.FAILED.value,
                                         priority=SubJobPriorityEnum.JOB_PRIORITY_2.value)
            raise LogDetailException(log_detail=err_log_detail)

    def backup_dest_check_if_need(self, ip, port, conn):
        # 租户备份不需要校验备份目的地址
        backup_job_level = self.check_backup_job_level(self._protect_object)
        if backup_job_level == OceanBaseBackupLevel.BACKUP_TENANT_LEVEL:
            log.info("tenant backup skip backup dest check")
            return
        # 检查backup_dest是否是持续挂载地址。不是：1关日志，2设置backup_dest，3开日志；如果是，直接返回成功
        try:
            self.check_backup_dest(ip, port)
        except Exception as ex:
            log.error(f"fail to check backup dest, :{ex}")
            update_sqlite_sub_job_status(conn=conn, updated_status=SubJobStatusForSqlite.FAILED.value,
                                         priority=SubJobPriorityEnum.JOB_PRIORITY_1.value)
            raise ex

    def restart_log_archive_if_need(self, ip, port):
        log.info("need to reopen log archive")
        sql_str = "ALTER SYSTEM NOARCHIVELOG;"
        self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        sel_str_query_result = self.backup_sql_cmd(OceanBaseSqlCmd.QUERY_LOG_STATUS)
        ret, output = self.exec_oceanbase_sql_with_expect(ip, port, sel_str_query_result,
                                                          OceanBaseQueryStatus.STOP.value, self._pid)
        if not ret:
            err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_EXEC_STOP_LOG_ARCHIVE_FAIL_LABEL,
                                       logInfoParam=[str(output)], logLevel=LogLevel.ERROR)
            raise LogDetailException(log_detail=err_log_detail)
        self.open_log_archive_and_wait_doing(ip, port)

    def check_backup_dest(self, ip, port):
        backup_dest_expected = os.path.join("/", self._cluster_name, self._cluster_id, "cluster")
        old_backup_dest = self.query_backup_destination(ip, port)
        # 判断backup_dest是否是持续挂载地址,如果是，直接返回成功
        if old_backup_dest == 'file://' + backup_dest_expected:
            log.info("old_backup_dest is OK")
            reopen_log_file = os.path.join(self._meta_area, f"reopen_log_{self._job_id}")
            # 如果挂载点已被解挂载（存在reopen_log_file），在restart_log_archive_if_need重开日志备份
            if os.path.exists(reopen_log_file):
                self.restart_log_archive_if_need(ip, port)
            return
        # 如果不是，则1停止日志备份，2设置backup_dest，3开启日志备份
        self.stop_log_archive_and_wait_stop(ip, port)
        self.set_backup_dest(ip, port, backup_dest_expected)
        self.open_log_archive_and_wait_doing(ip, port)
        log.info("reset archivelog to X8000 succeeded")

    def set_backup_dest(self, ip, port, backup_dest_expected):
        log.warn("start to set backup dest")
        # 上报警告提示码ErrorCode.WARN_SET_BACKUP_DEST
        sql_str = F"ALTER SYSTEM SET backup_dest='file://{backup_dest_expected}';"
        ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        if not ret:
            err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_SET_BACKUP_DEST_FAIL_LABEL,
                                       logInfoParam=[str(output)], logLevel=LogLevel.ERROR)
            raise LogDetailException(log_detail=err_log_detail)
        log.info(f"reset backup dest success")
        return True

    def stop_log_archive_and_wait_stop(self, ip, port):
        log.warn("start to stop log archive")
        # 上报警告提示码ErrorCode.WARN_STOP_ARCHIVELOG
        sql_str = "ALTER SYSTEM CANCEL all BACKUP force;"
        ret_exec, output_exec = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        if not ret_exec:
            err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_EXEC_STOP_LOG_ARCHIVE_FAIL_LABEL,
                                       logInfoParam=[str(output_exec)], logLevel=LogLevel.ERROR)
            raise LogDetailException(log_detail=err_log_detail)

        sql_str_query_result = self.backup_sql_cmd(OceanBaseSqlCmd.QUERY_LOG_STATUS)
        ret_query, output_query = self.exec_oceanbase_sql_with_expect(ip, port, sql_str_query_result,
                                                                      OceanBaseQueryStatus.STOP.value, self._pid)
        if not ret_query:
            err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_EXEC_STOP_LOG_ARCHIVE_FAIL_LABEL,
                                       logInfoParam=[str(output_query)], logLevel=LogLevel.ERROR)
            raise LogDetailException(log_detail=err_log_detail)
        log.info("stop log archive succeed")
        return True

    def open_log_archive_and_wait_doing(self, ip, port):
        log.warn("start to open log archive")
        # 上报警告提示码ErrorCode.WARN_NO_ARCHIVELOG_AND_OPEN
        sql_str = "ALTER SYSTEM ARCHIVELOG;"
        ret_exec, output_exec = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        if not ret_exec:
            err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_EXEC_OPEN_LOG_ARCHIVE_FAIL_LABEL,
                                       logInfoParam=[str(output_exec)], logLevel=LogLevel.ERROR)
            raise LogDetailException(log_detail=err_log_detail)

        sql_str_query_result = self.backup_sql_cmd(OceanBaseSqlCmd.QUERY_LOG_STATUS)
        ret_query, output_query = self.exec_oceanbase_sql_with_expect(ip, port, sql_str_query_result,
                                                                      OceanBaseQueryStatus.DOING.value, self._pid)
        if not ret_query:
            err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_EXEC_OPEN_LOG_ARCHIVE_FAIL_LABEL,
                                       logInfoParam=[str(output_query)], logLevel=LogLevel.ERROR)
            raise LogDetailException(log_detail=err_log_detail)
        log.info("open log archive succeed")
        return True

    def check_log_status(self, ip, port):
        sql_str = self.backup_sql_cmd(cmd=OceanBaseSqlCmd.QUERY_LOG_STATUS)
        ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        retry_time = 100
        while not self.check_sql_output(output, OceanBaseQueryStatus.DOING):
            if not ret or retry_time < 0:
                log.error("Fail to check log status")
                return False
            if (OceanBaseQueryStatus.BEGINNING,) in output:
                log.info(f"status BEGINNING in {output}, wait log archive DOING")
                time.sleep(10)
                retry_time -= 1
                ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
            else:
                log.error("log status is not DOING, open it and retry")
                return False
        log.info("log status is DOING now, task continues")
        return True

    def exec_data_backup_sub_job(self):
        log.info("start to exec_data_backup_sub_job")
        ret = self.pre_check_before_sub_job(SubJobPriorityEnum.JOB_PRIORITY_2)
        if isinstance(ret, bool):
            return ret, 0, 0
        conn = ret
        ip, port = self.get_enter_database_ip_port(self._protect_env)
        if not self.check_obclient_connection(ip, port, self._pid):
            if not update_sqlite_sub_job_status(conn=conn, updated_status=SubJobStatusForSqlite.RETRY.value,
                                                priority=SubJobPriorityEnum.JOB_PRIORITY_2.value):
                err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_ALL_OBCLIENT_OFFLINE_LABEL,
                                           logInfoParam=[], logLevel=LogLevel.ERROR)
                raise LogDetailException(log_detail=err_log_detail)
            log.debug("local node fail to connect observer, update sqlite status to RETRY")
            return True, 0, 0
        # 获取tenant信息
        self.build_tenant_id_name_info(ip, port)
        # 上报备份速度
        self._job_status = SubJobStatusEnum.RUNNING
        time_start = int((time.time()))
        log.info("start to upload speed")
        data_path = self.get_data_path()
        progress_thread = threading.Thread(name='pre_progress', target=self.upload_backup_progress,
                                           args=(data_path,))
        progress_thread.daemon = True
        progress_thread.start()

        # 查询备份的库表信息
        self.sqlite_for_display(ip, port)
        log.info("start thread for creating sqlite")
        # 执行备份命令
        self.exec_data_backup_cmd(ip, port, conn)

        sql_str = self.backup_sql_cmd(cmd=OceanBaseSqlCmd.MAX_BS_KEY_STATUS,
                                      tenant_id_list=self._tenant_id_list,
                                      bs_key=self._bs_key)
        expect_output = OceanBaseQueryStatus.SUCCESS
        ret, output = self.exec_oceanbase_sql_with_expect(ip, port, sql_str, expect_output, self._pid)
        if not ret:
            log.error("exec sql and the output of query is not SUCCESS")
            update_sqlite_sub_job_status(conn=conn, updated_status=SubJobStatusForSqlite.FAILED.value,
                                         priority=SubJobPriorityEnum.JOB_PRIORITY_2.value)
            err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_EXEC_BACKUP_SUB_JOB_FAIL_LABEL,
                                       logInfoParam=[str(output)], logLevel=LogLevel.ERROR)
            raise LogDetailException(log_detail=err_log_detail)
        update_sqlite_sub_job_status(conn=conn, updated_status=SubJobStatusForSqlite.SUCCESS.value,
                                     priority=SubJobPriorityEnum.JOB_PRIORITY_2.value)

        self._job_status = SubJobStatusEnum.COMPLETED
        # 此处关闭线程
        log.info("end to upload speed")
        progress_thread.join()
        # 获取副本大小和速度
        data_size, speed = self.get_data_size_and_speed(ip, port)
        # 上报副本信息
        self.upload_data_copy(ip, port)
        return True, data_size, speed

    def get_data_path(self):
        data_path = self._persistent_mount
        backup_job_level = self.check_backup_job_level(self._protect_object)
        if backup_job_level == OceanBaseBackupLevel.BACKUP_CLUSTER_LEVEL:
            data_path = os.path.join(data_path, "cluster")
        elif backup_job_level == OceanBaseBackupLevel.BACKUP_TENANT_LEVEL:
            data_path = os.path.join(self._data_area, self._copy_id)
        return data_path

    def get_data_size_and_speed(self, ip, port):
        sql_str = f'SELECT OUTPUT_BYTES_DISPLAY, OUTPUT_RATE_BYTES_DISPLAY FROM oceanbase.CDB_OB_BACKUP_SET_FILES ' \
                  f'WHERE TENANT_ID=1 AND BS_KEY={self._bs_key}'
        ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        if not ret:
            log.error("fail to get size and speed")
            return 0, 0
        log.info(f"get_data_size_and_speed output is {output}")
        pattern = r'\d+\.?\d*'
        size_unit = re.sub(pattern, '', output[0][0])
        speed_unit = re.sub(pattern, '', output[0][1])
        size_unit_value = {"MB": 1024, "GB": 1024 * 1024, "TB": 1024 * 1024 * 1024, "PB": 1024 * 1024 * 1024 * 1024}
        speed_unit_value = \
            {"MB/S": 1024, "GB/S": 1024 * 1024, "TB/S": 1024 * 1024 * 1024, "PB/S": 1024 * 1024 * 1024 * 1024}
        size_unit_coefficient = size_unit_value.get(size_unit)
        speed_unit_coefficient = speed_unit_value.get(speed_unit)
        size = str_to_float(output[0][0].replace(size_unit, "").strip()) * size_unit_coefficient
        speed = str_to_float(output[0][1].replace(speed_unit, "").strip()) * speed_unit_coefficient
        return size, speed

    def upload_data_copy(self, ip, port):
        self._incarnation_id = self.query_incarnation_id(ip, port)
        self._backup_time = self.query_backup_time(ip, port)
        min_max_next_time = min(self.query_log_max_next_time().values())
        while min_max_next_time < self._backup_time:
            # 等待日志备份的max_next_time 追平 备份的completion_time
            log.info(f"wait log archive")
            log.info(f"now max_next_time {min_max_next_time}, completion time is {self._backup_time}")
            time.sleep(30)
            min_max_next_time = min(self.query_log_max_next_time().values())
        log.info(f"min_max_next_time is {min_max_next_time}, backup_time is {self._backup_time}")
        # 如果是全量备份，需要清除之前的副本
        self.clear_backup_set(ip, port)
        self.report_cluster_info()

    def clear_backup_set(self, ip, port):
        backup_job_level = self.check_backup_job_level(self._protect_object)
        if backup_job_level == OceanBaseBackupLevel.BACKUP_TENANT_LEVEL:
            return
        if not self._backup_type == BackupTypeEnum.FULL_BACKUP:
            return
        # 去meta仓里看看，有没有记录上次的备份的最后的bs_key，如果没有就不去清
        # 只清除我们自己备份的副本，客户别的备份我们不管，如果清理可能会一直报错
        cluster_full_backup_bs_key_path = os.path.join(self._meta_area, "clusterFullBackupBskey")
        if not os.path.exists(cluster_full_backup_bs_key_path):
            return
        last_bs_key = read_tmp_json_file(cluster_full_backup_bs_key_path).get("bs_key")
        sql_str = self.backup_sql_cmd(cmd=OceanBaseSqlCmd.QUERY_OLD_BACKUP_SET, bs_key=self._bs_key,
                                      last_bs_key=last_bs_key)
        ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        if not ret or not output:
            log.warn("query old backup set ids failed")
            return
        # 原来删除失败重试100次，根据现网观察，100次也成功不了，所以降低重试次数为5次
        for bs_key in output:
            backup_set_id = bs_key[0]
            clear_cnt = 1
            clear_sql = f"ALTER SYSTEM DELETE BACKUPSET {backup_set_id}"
            clear_ret, output = self.exec_oceanbase_sql(ip, port, clear_sql, self._pid)
            time.sleep(15)
            while not clear_ret:
                if clear_cnt > 5:
                    log.error("clear backup set over 5 times")
                    return
                clear_cnt += 1
                log.warn("wait last clear task finish")
                clear_ret, output = self.exec_oceanbase_sql(ip, port, clear_sql, self._pid)
                time.sleep(15)
        return

    def build_tenant_id_name_info(self, ip, port):
        backup_job_level = self.check_backup_job_level(self._protect_object)
        if backup_job_level == OceanBaseBackupLevel.BACKUP_CLUSTER_LEVEL:
            self._tenant_name_list, self._tenant_id_list, self._tenant_name_id_list = \
                self.query_tenant_name_id_list(ip, port)
        else:  # 租户集备份：
            self._tenant_name_list = self.get_tenant_name_list(self._protect_object)
            self._tenant_id_list, self._tenant_name_id_list = self.query_tenant_id_list_by_name(ip, port,
                                                                                                self._tenant_name_list)
            self._tenant_id_list.insert(0, 1)

    def exec_data_backup_cmd(self, ip, port, conn):
        backup_job_level = self.check_backup_job_level(self._protect_object)
        if backup_job_level == OceanBaseBackupLevel.BACKUP_CLUSTER_LEVEL:
            if self._backup_type == BackupTypeEnum.INCRE_BACKUP:
                sql_str = self.backup_sql_cmd(OceanBaseSqlCmd.CLUSTER_INCRE_BACKUP)
            else:
                sql_str = self.backup_sql_cmd(OceanBaseSqlCmd.CLUSTER_FULL_BACKUP)
            ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        elif backup_job_level == OceanBaseBackupLevel.BACKUP_TENANT_LEVEL:
            tenant_name_list_str = ','.join(self._tenant_name_list)
            backup_destination = os.path.join(self._data_area, self._copy_id)
            cmd_str_list = [f"mkdir -p {backup_destination}", f"chown -R admin:admin {self._data_area}"]
            execute_cmd_list(cmd_str_list)
            sql_str = self.backup_sql_cmd(OceanBaseSqlCmd.TENANT_FULL_BACKUP,
                                          tenant_name_list=tenant_name_list_str,
                                          backup_destination=f'file://{backup_destination}')
            ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        if not ret:
            log.error("fail to exec backup sql statement")
            update_sqlite_sub_job_status(conn=conn, updated_status=SubJobStatusForSqlite.FAILED.value,
                                         priority=SubJobPriorityEnum.JOB_PRIORITY_2.value)
            err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_EXEC_BACKUP_SUB_JOB_FAIL_LABEL,
                                       logInfoParam=[str(output)], logLevel=LogLevel.ERROR)
            raise LogDetailException(log_detail=err_log_detail)
        time.sleep(self._query_time_interval)  # sleep一段时间，太快查询会查不到结果
        sql_str = f'SELECT TENANT_ID, BS_KEY FROM oceanbase.CDB_OB_BACKUP_SET_FILES ' \
                  f'WHERE TENANT_ID = {self._tenant_id_list[1]}'
        ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        if not ret:
            log.error("fail to query max bs_key")
            update_sqlite_sub_job_status(conn=conn, updated_status=SubJobStatusForSqlite.FAILED.value,
                                         priority=SubJobPriorityEnum.JOB_PRIORITY_2.value)
            err_log_detail = LogDetail(logInfo=OceanBaseReportLabel.BACKUP_EXEC_BACKUP_SUB_JOB_FAIL_LABEL,
                                       logInfoParam=[str(output)], logLevel=LogLevel.ERROR)
            raise LogDetailException(log_detail=err_log_detail)
        self._bs_key = max(output)[-1]
        if (backup_job_level == OceanBaseBackupLevel.BACKUP_CLUSTER_LEVEL and
                self._backup_type == BackupTypeEnum.FULL_BACKUP.value):
            cluster_full_backup_bs_key_path = os.path.join(self._cache_area, "clusterFullBackupBskey")
            output_execution_result_ex(cluster_full_backup_bs_key_path, {"bs_key": self._bs_key})
        log.info(f'the bs_key for this backup is : {self._bs_key}')
        return True

    def report_cluster_info(self):
        if self._backup_type == BackupTypeEnum.FULL_BACKUP:
            last_copy_id_file = os.path.join(self._meta_area, "lastCopyId", "lastCopyId")
            last_copy_id_path = os.path.join(self._meta_area, "lastCopyId")
            log.info(f"current task copyId: {self._copy_id}")
            if not os.path.exists(last_copy_id_path):
                os.makedirs(last_copy_id_path)
            output_execution_result_ex(last_copy_id_file, self._copy_id)

        # 保存当前实例的集群信息，用于恢复时使用
        cluster_info = self._protect_env.get("extendInfo", {}).get("clusterInfo")
        copy_info = {
            "cluster_info": cluster_info,
            "backup_time": self._backup_time
        }
        log.info(f"upload backup_time is {self._backup_time}")
        copy_info_file = os.path.join(self._meta_area, self._copy_id, "copy_info")
        copy_info_path = os.path.join(self._meta_area, self._copy_id)
        if not os.path.exists(copy_info_path):
            os.makedirs(copy_info_path)
        output_execution_result_ex(copy_info_file, copy_info)
        log.info("step2-6 end to sub_job_exec")
        self.report_copy_info()

    def exec_log_copy_sub_job(self):
        # log_copy开始前， 准备所需的信息
        last_copy_info = self.get_log_backup_last_copy_info()
        if not last_copy_info:
            log.error(f"Fail to get previous copy info")
            return False, 0, 0
        self.prepare_log_copy_info(last_copy_info)
        self._cluster_name, self._cluster_id = self.get_cluster_name_and_id(self._protect_env)
        # 组装clog_source, copy_to_path, period，并执行copy_log_incremental
        mount_point = os.path.join("/", self._cluster_name, self._cluster_id, "cluster")
        basic_path = os.path.join(self._cluster_name, self._cluster_id, f'incarnation_{self._incarnation_id}')
        clog_source = os.path.join(mount_point, basic_path)
        copy_to_path = os.path.join(self._log_area, self._copy_id)
        last_backup_time, last_copy_id = self.get_last_backup_time_and_id(last_copy_info)
        max_next_time = datetime.datetime.strptime(self._max_next_time, "%Y-%m-%d %H:%M:%S")
        max_next_time_stamp = int(max_next_time.timestamp())
        period = self.build_period_for_log_copy(int(last_backup_time), max_next_time_stamp)
        # 在log仓创建每个tenant_id的文件夹
        log_copy_tenant_list = self.get_log_copy_tenant_list(clog_source)
        self.mkdir_copy_dest(log_copy_tenant_list)
        log.info("start to upload speed")
        self._job_status = SubJobStatusEnum.RUNNING
        time_start = int((time.time()))
        progress_thread = threading.Thread(name='pre_progress', target=self.upload_backup_progress,
                                           args=(self._log_area,))
        progress_thread.daemon = True
        progress_thread.start()

        self.copy_log_incremental(log_copy_tenant_list, clog_source, copy_to_path, period)

        self._job_status = SubJobStatusEnum.COMPLETED
        try:
            os.remove(os.path.join(self._cache_area, f'speed_{self._job_id}'))
            log.info("remove speed file and end speed report")
        except Exception:
            log.error("fail to remove speed_file")
        # 此处关闭线程
        log.info("end to upload speed")
        progress_thread.join()
        time_completed = int((time.time()))
        time_diff = time_completed - time_start + 1

        data_size_total = get_dir_size(os.path.join(self._log_area, self._job_id))
        try:
            speed = data_size_total / time_diff
        except ZeroDivisionError:
            return False, 0, 0
        log.info(f"begin to report log copy")
        self.report_copy_info_binlog(last_backup_time, last_copy_id)
        return True, data_size_total, speed

    def prepare_log_copy_info(self, last_copy_info):
        # log_copy开始前， 准备所需的信息
        ip, port = self.get_enter_database_ip_port(self._protect_env)
        self._incarnation_id = self.query_incarnation_id(ip, port)
        tenant_list = last_copy_info.get("extendInfo", {}).get("tenant_list")
        for tenant in tenant_list:
            tenant_name = tenant.get("name")
            # 根据tenant_name查询tenant_id, 如果查到则更新tenant_id, 查不到则保持原id
            tenant_id = self.query_tenant_id_by_name(ip, port, tenant_name)
            if not tenant_id:
                tenant_id = tenant.get("id")
            self._tenant_name_list.append(tenant_name)
            self._tenant_id_list.append(tenant_id)
            tenant_name_id = {"name": tenant_name, "id": tenant_id}
            self._tenant_name_id_list.append(tenant_name_id)
        log.info(f"self._tenant_name_id_list is {self._tenant_name_id_list}")
        max_next_time_dict = self.query_log_max_next_time()
        log.info(f'sub_job_1 for log: report max_next_time_dict : {max_next_time_dict}')
        min_max_next_time = min(max_next_time_dict.values())
        self._max_next_time = str(min_max_next_time).split('.')[0]

    def mkdir_copy_dest(self, tenant_id_list):
        repo_path = self._log_area
        for tenant_id in tenant_id_list:
            copy_dest = os.path.join(repo_path, self._copy_id, f'{tenant_id}')
            cmd_str = f'mkdir -p {copy_dest}'
            return_code, out_info, err_info = execute_cmd(cmd_str)
            if return_code != CMDResult.SUCCESS:
                log.error(f"The execute mkdir_copy_dest cmd failed!")
                return False
            ret = os.path.exists(copy_dest)
            if not ret:
                log.error(f'mkdir_copy_dest failed')
                return False
        return True

    def exec_backup_post_job(self):
        log.info('post job begins')
        cluster_full_backup_bs_key_path = os.path.join(self._cache_area, "clusterFullBackupBskey")
        if os.path.exists(cluster_full_backup_bs_key_path):
            cluster_full_backup_bs_key = read_tmp_json_file(cluster_full_backup_bs_key_path)
            os.remove(cluster_full_backup_bs_key_path)
            cluster_full_backup_bs_key_path = os.path.join(self._meta_area, "clusterFullBackupBskey")
            output_execution_result_ex(cluster_full_backup_bs_key_path, cluster_full_backup_bs_key)
        sqlite_file_path = os.path.join(self._meta_area, "sqlite_file")
        if not self.remove_tmp_file(sqlite_file_path):
            return False
        reopen_log_file = os.path.join(self._meta_area, f"reopen_log_{self._job_id}")
        if os.path.exists(reopen_log_file):
            remove_dir(reopen_log_file)
            log.info(f"reopen_log_file existed and had been removed")
        return True

    def query_tenant_name_id_list(self, observer_ip, observer_port):
        sql_str = "SELECT tenant_name, tenant_id FROM oceanbase.gv$tenant"
        ret, output = self.exec_oceanbase_sql(observer_ip, observer_port, sql_str, self._pid)
        tenant_name_list = []
        tenant_id_list = []
        tenant_name_id_list = list(output)
        for tenant in output:
            tenant_name_list.append(tenant[0])
            tenant_id_list.append(tenant[1])
        return tenant_name_list, tenant_id_list, tenant_name_id_list[1:]

    def query_tenant_id_list_by_name(self, observer_ip, observer_port, tenant_name_list):
        sql_str = self.backup_sql_cmd(OceanBaseSqlCmd.QUERY_TENANT_ID_LIST_BY_NAME,
                                      tenant_name_list=tenant_name_list)
        ret, output = self.exec_oceanbase_sql(observer_ip, observer_port, sql_str, self._pid)
        tenant_id_list = []
        for tenant in output:
            tenant_id_list.append(tenant[1])
        tenant_name_id_list = list(output)
        return tenant_id_list, tenant_name_id_list

    def query_backup_destination(self, ip, port):
        sql_str = BackUp.backup_sql_cmd(OceanBaseSqlCmd.QUERY_BACKUP_DESTINATION)
        log.info("query_backup_destination")
        ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        if not ret:
            log.error("fail to query backup_destination")
            return ""
        if not output:
            return ""
        try:
            dest = output[0][0]
            return dest
        except KeyError:
            return ""

    def query_incarnation_id(self, ip, port):
        sql_str = BackUp.backup_sql_cmd(OceanBaseSqlCmd.QUERY_INCARNATION_ID)
        ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        if len(set(output)) != 1:
            log.error(f"query_incarnation_id return {output}, not single incarnation id")
            return False
        try:
            return output[0][0]
        except KeyError:
            return False

    def query_backup_time(self, ip, port):
        sql_str = BackUp.backup_sql_cmd(OceanBaseSqlCmd.QUERY_BACKUP_TIME,
                                        bs_key=self._bs_key, tenant_id_list=self._tenant_id_list)
        ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        if len(set(output)) != 1:
            log.error(f"backup_backup_time is different: {output}")
            return False
        try:
            output_time = str(output[0][0]).split('.')[0]
            return output_time
        except KeyError:
            return False

    def query_log_max_next_time(self):
        sql_str = BackUp.backup_sql_cmd(OceanBaseSqlCmd.QUERY_LOG_MAX_NEXT_TIME,
                                        tenant_id_list=self._tenant_id_list)
        ip, port = self.get_enter_database_ip_port(self._protect_env)
        ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        if not ret:
            log.error("fail to get log max_next_time")
            return {}
        dict_max_next_time = dict(output)
        return dict_max_next_time

    def query_tenant_del_time(self, ip, port, tenant_id):
        sql_str = f"select gmt_modified from oceanbase.__all_tenant_history " \
                  f"where tenant_id={tenant_id} and is_deleted=1;"
        ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        if len(set(output)) == 0:
            return ""
        try:
            return str(output[0][0]).split('.')[0]
        except KeyError:
            return ""

    def query_tenant_id_by_name(self, observer_ip, observer_port, tenant_name):
        sql_str = f"SELECT tenant_id FROM oceanbase.gv$tenant WHERE tenant_name = '{tenant_name}'"
        ret, output = self.exec_oceanbase_sql(observer_ip, observer_port, sql_str, self._pid)
        if not ret:
            log.error("fail to query backup_destination")
            return ""
        if not output:
            log.error(f"query by {tenant_name} return empty")
            return ""
        try:
            return output[0][0]
        except KeyError:
            return ""

    def report_copy_info_binlog(self, last_backup_time, last_copy_id):
        log.info(f"Start to report_copy_info_binlog.")
        extend_info = self.build_log_backup_extend_info(last_backup_time, last_copy_id)
        copy = Copy(repositories=[], extendInfo=extend_info)
        copy_info = ReportCopyInfoModel(copy=copy, jobId=self._job_id).dict(by_alias=True)
        try:
            exec_rc_tool_cmd(self._job_id, RpcParamKey.REPORT_COPY_INFO, copy_info)
        except Exception as err_info:
            log.error(f"Report copy info fail.err: {err_info}.")
            return False
        log.info(f"Report copy info success {copy_info}.")
        return True

    def build_log_backup_extend_info(self, last_backup_time, last_copy_id):
        """
        组装日志副本上报信息
        :return:
        """
        if not last_backup_time or not last_copy_id:
            return {}
        max_next_time = datetime.datetime.strptime(self._max_next_time, "%Y-%m-%d %H:%M:%S")
        max_next_time_stamp = max_next_time.timestamp()
        end_time = int(max_next_time_stamp)

        tenant_list = []
        for tenant_name_id in self._tenant_name_id_list:
            # 获取每个租户的可恢复时间段
            tenant_end_time = end_time
            tenant_id = tenant_name_id.get("id")
            ip, port = self.get_enter_database_ip_port(self._protect_env)
            tenant_del_time = self.query_tenant_del_time(ip, port, tenant_id)
            is_delete = False
            if tenant_del_time:
                log.info(f"tenant {tenant_id} delete time is {tenant_del_time}")
                tenant_end_time = datetime.datetime.strptime(tenant_del_time, "%Y-%m-%d %H:%M:%S")
                tenant_end_time = int(tenant_end_time.timestamp())
                is_delete = True
            dict_tenant = {
                "name": tenant_name_id.get("name"), "id": tenant_id, "beginTime": last_backup_time,
                "endTime": tenant_end_time,
                "idDeleted": is_delete
            }
            tenant_list.append(dict_tenant)

        extend_info = {
            "backupTime": last_backup_time,
            "beginTime": last_backup_time,
            "endTime": end_time,
            "beginSCN": None,
            "endSCN": None,
            "associatedCopies": [],
            "logDirName": self._log_area,
            "tenant_list": tenant_list,
            "copy_id": self._copy_id
        }
        return extend_info

    def check_backup_job_type(self):
        # 当此次任务是全量备份，直接返回True
        # 当此次任务是增量备份，且之前没做过全量备份，需要转全量
        # 当此次任务是日志备份：无任何副本，则任务失败
        log.info(f'step 2-1: start execute check_backup_job_type, pid: {self._pid}, job_id:{self._job_id}')
        backup_type = self._backup_type
        log.info(f"check backup_type is {backup_type}")
        if not backup_type:
            return False
        if backup_type == BackupTypeEnum.FULL_BACKUP:
            return True

        if not self.check_backup_type():
            log.error(f"check_last_copy_is_null.")
            if backup_type == BackupTypeEnum.INCRE_BACKUP:
                response = ActionResponse(code=ExecuteResultEnum.INTERNAL_ERROR,
                                          bodyErr=ErrorCode.ERROR_INCREMENT_TO_FULL,
                                          message="Can not apply this type backup job")
                output_result_file(self._pid, response.dict(by_alias=True))
                log.info(f"change backup_type increment to full")
                return False
            elif backup_type == BackupTypeEnum.LOG_BACKUP:
                # 日志备份，无任何副本，则任务失败
                response = ActionResponse(code=ExecuteResultEnum.INTERNAL_ERROR,
                                          message="Can not apply this type backup job")
                output_result_file(self._pid, response.dict(by_alias=True))
                log.info(f"Can not apply binlog backup")
                return False
        response = ActionResponse(code=ExecuteResultEnum.SUCCESS)
        output_result_file(self._pid, response.dict(by_alias=True))
        log.info(f'step 2-1: finish execute check_backup_job_type, pid: {self._pid}, job_id:{self._job_id}')
        return True

    def check_backup_type(self):
        # 读取last_copy_info
        if self._backup_type == BackupTypeEnum.LOG_BACKUP:
            last_copy_info = self.get_last_copy_info(4)
        else:
            last_copy_info = self.get_last_copy_info(1)
        last_copy_info_file = os.path.join(self._meta_area, "lastCopyInfo", f"jobid_{self._job_id}")
        last_copy_info_path = os.path.join(self._meta_area, "lastCopyInfo")
        if not os.path.exists(last_copy_info_path):
            os.makedirs(last_copy_info_path)
        if last_copy_info:
            output_execution_result_ex(last_copy_info_file, last_copy_info)
            log.info(f"save last copy info to {last_copy_info_file} successfully")
            return True
        # 检查meta中记录的上次备份副本集群集结构与当前集群结构是否相同
        # 检查meta中记录的上次备份租户集结构和当前租户集结构是否相同
        return False

    def get_last_copy_info(self, copy_type: int):
        # 获取上次数据备份副本信息
        log.info("start get_last_copy_info")
        last_copy_type = LastCopyType.last_copy_type_dict.get(copy_type)
        input_param = {
            RpcParamKey.APPLICATION: self._json_param.get("job", {}).get("protectObject"),
            RpcParamKey.TYPES: last_copy_type,
            RpcParamKey.COPY_ID: "",
            RpcParamKey.JOB_ID: self._job_id
        }
        try:
            result = invoke_rpc_tool_interface(self._job_id, RpcParamKey.QUERY_PREVIOUS_CPOY, input_param)
        except Exception as err_info:
            log.error(f"Get last copy info fail.{err_info}")
            return {}
        return result

    def get_log_backup_last_copy_info(self):
        # 以上次日志备份作为起点，如果找不到上一次日志备份，则以上一次数据备份作为起点
        log.info("start get_log_backup_last_copy_info")
        last_copy_info = self.get_last_copy_info(3)
        if not last_copy_info:
            log.warning("This is the first log backup.")
            last_copy_info = self.get_last_copy_info(1)
        log.info(f"last copy info is {last_copy_info}")
        return last_copy_info

    def report_copy_info(self):
        backup_time = self._backup_time
        backup_time = datetime.datetime.strptime(backup_time, "%Y-%m-%d %H:%M:%S")
        backup_time = backup_time.timestamp()
        tenant_list = []
        for tenant in self._tenant_name_id_list:
            dict_tenant = {"name": tenant[0], "id": tenant[1]}
            tenant_list.append(dict_tenant)

        extend_info = {
            "copy_id": self._copy_id,
            "backup_time": backup_time,
            "tenant_list": tenant_list
        }
        copy = Copy(repositories=[], extendInfo=extend_info)
        copy_info = ReportCopyInfoModel(copy=copy, jobId=self._job_id).dict(by_alias=True)
        invoke_rpc_tool_interface(self._job_id, RpcParamKey.REPORT_COPY_INFO, copy_info)
        log.debug(f"Finish report copy_info!")

    def upload_backup_progress(self, data_path):
        log.info(f"query backup speed and size")
        report_progress_thread = threading.Thread(name='report_progress', target=self.report_backup_progress)
        report_progress_thread.daemon = True
        report_progress_thread.start()
        speed_file = os.path.join(self._cache_area, f'speed_{self._job_id}')
        time_init = int((time.time()))
        speed_info = {
            'data_size': int(get_dir_size(data_path)),
            'time': time_init
        }
        log.info(f'initial speed info is {speed_info}')
        output_execution_result_ex(speed_file, speed_info)
        process_file = os.path.join(self._cache_area, f"BackupProgress_{self._job_id}")
        while self._job_status == SubJobStatusEnum.RUNNING:
            log.info(f"progress RUNNING")
            speed, data_size_diff = self.calculate_speed(speed_file, data_path)
            if not os.path.exists(speed_file):
                log.info("speed file has been removed, break")
                break
            log.info(f'speed, data_size_diff is {speed, data_size_diff}')
            sub_job_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                         taskStatus=SubJobStatusEnum.RUNNING.value,
                                         speed=speed, logDetail=[], progress=20)

            progress_dict = sub_job_dict.dict(by_alias=True)
            output_execution_result_ex(process_file, progress_dict)
            time.sleep(self._query_time_interval)
        touch_file(os.path.join(self._cache_area, f"BackupComputeOverFlag_{self._job_id}"))
        report_progress_thread.join()
        log.info("sub thread ends here")

    def report_backup_progress(self):
        log.info("report start.")
        sub_job_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                     taskStatus=SubJobStatusEnum.RUNNING.value, logDetail=[], progress=20)
        progress_dict = sub_job_dict.dict(by_alias=True)
        process_file = os.path.join(self._cache_area, f"BackupProgress_{self._job_id}")
        # 等上面的upload_backup_progress线程结束了，这边上报才能结束掉
        backup_compute_over_flag_file = os.path.join(self._cache_area, f"BackupComputeOverFlag_{self._job_id}")
        while not os.path.exists(backup_compute_over_flag_file):
            if os.path.exists(process_file):
                log.debug(f"process file exists.")
                try:
                    progress_dict = read_tmp_json_file(process_file)
                except Exception as ex:
                    log.error(ex, exc_info=True)
            report_job_details(self._job_id, progress_dict)
            time.sleep(self._query_time_interval)
        if os.path.isfile(process_file):
            if not su_exec_rm_cmd(process_file):
                log.warn(f"Fail to remove {process_file}.")
        if os.path.isfile(backup_compute_over_flag_file):
            if not su_exec_rm_cmd(backup_compute_over_flag_file):
                log.warn(f"Fail to remove {backup_compute_over_flag_file}.")
        log.info("report end.")

    def check_obclient_connection(self, ip, port, pid):
        try:
            self.get_db_session(ip, port, pid)
            return True
        except Exception:
            return False

    def sqlite_for_display(self, observer_ip, observer_port):
        path = os.path.join(self._meta_area)
        tenant_name_list = self._tenant_name_list
        # 查出tenant中所有能查到的database与table
        query_database_str = self.backup_sql_cmd(OceanBaseSqlCmd.QUERY_DATABASE_FOR_DISPLAY,
                                                 tenant_id_list=self._tenant_id_list)
        query_table_str = self.backup_sql_cmd(OceanBaseSqlCmd.QUERY_TABLE_FOR_DISPLAY,
                                              tenant_id_list=self._tenant_id_list)
        ret_database, output_database = self.exec_oceanbase_sql(observer_ip, observer_port,
                                                                query_database_str, self._pid)
        ret_table, output_table = self.exec_oceanbase_sql(observer_ip, observer_port,
                                                          query_table_str, self._pid)
        log.info(f"result query from database is {output_database, output_table}")
        if not ret_database or not ret_table:
            log.error("query database structure failed")
            return
        try:
            # 将查出的database与table写入sqlite文件
            OceanBaseSqliteService.write_sqlite(path, tenant_name_list, output_database, output_table)
        except Exception as exception:
            log.error(f'fail to write sqlite for display, error is {exception}')
            return
        log.info("sqlite_for_display succeeded")

    def update_log_archive_interval(self, ip, port):
        # 获取用户设置的log_archive_interval, 若获取失败则设置为60
        log_archive_interval = self._protect_env.get("extendInfo", {}).get("logArchiveInterval", "60")
        sql_str = "SELECT VALUE FROM oceanbase. __all_sys_parameter WHERE name='backup_dest_option'"
        ret, output = self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
        log.info(f"old backup_dest_option is {output}")
        if not ret:
            log.error("Fail to query backup_dest_option")
            return False
        if not output:
            # 如果backup_dest_option没有设置，则直接执行命令：log_archive_checkpoint_interval={log_archive_interval}s
            sql_str = f"alter system set backup_dest_option='log_archive_checkpoint_interval={log_archive_interval}s'"
            self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
            return True
        try:
            old_backup_dest_option = output[0][0]
        except KeyError:
            log.error("Fail to update backup_dest_option")
            return False
        if "log_archive_checkpoint_interval" not in old_backup_dest_option:
            # 如果backup_dest_option设置了但是没有设置log_archive_checkpoint_interval：
            # 则执行命令：原来的配置+log_archive_checkpoint_interval=30s
            backup_dest_option = old_backup_dest_option + f"log_archive_checkpoint_interval={log_archive_interval}s"
            sql_str = f"alter system set backup_dest_option='{backup_dest_option}'"
            self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
            return True
        # 如果backup_dest_option设置了也设置了log_archive_checkpoint_interval：
        # 则执行命令：原来的配置（其中log_archive_checkpoint_interval改为=30s）
        old_backup_dest_option_list = old_backup_dest_option.split("&")
        log.info(f"old_backup_dest_option_list is {old_backup_dest_option_list}")
        for option_num, option in enumerate(old_backup_dest_option_list):
            if "log_archive_checkpoint_interval" in option:
                log.info(f"this option is {option}")
                old_backup_dest_option_list[option_num] = f"log_archive_checkpoint_interval={log_archive_interval}s"
                backup_dest_option = "&".join(old_backup_dest_option_list)
                log.info(f"option_str is {backup_dest_option}")
                sql_str = f"alter system set backup_dest_option='{backup_dest_option}'"
                self.exec_oceanbase_sql(ip, port, sql_str, self._pid)
                log.info("succeed to update backup_dest_option")
                return True
        log.error("fail to find log_archive_checkpoint_interval option")
        return False

    def get_log_copy_tenant_list(self, clog_source):
        tenant_id_list = []
        for sub_dir in os.listdir(clog_source):
            log.info(f"sub_dir is {sub_dir}")
            sub_dir_path = os.path.join(clog_source, sub_dir)
            log.info(f"sub_dir_path is {sub_dir_path}")
            if not os.path.isdir(sub_dir_path):
                continue
            if not sub_dir.isdigit():
                continue
            # 过滤掉租户1
            if sub_dir == "1":
                continue
            tenant_id_list.append(sub_dir)
        log.info(f"get log copy tenant list {tenant_id_list}, pid: {self._pid}")
        return tenant_id_list
