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
import os
import time

from common.common import execute_cmd, is_clone_file_system
from common.const import SubJobStatusEnum, RepositoryDataTypeEnum
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import exec_mkdir_cmd, exec_cp_cmd, exec_mv_cmd
from mysql import log
from mysql.src.common.constant import MySQLJsonConstant, MySQLRestoreStep, ExecCmdResult, MySQLStrConstant
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.execute_cmd import mysql_backup_files, exec_sql, safe_get_environ
from mysql.src.common.parse_parafile import MasterInfoParseParam
from mysql.src.protect_mysql_instance_restore import MysqlInstanceRestore
from mysql.src.utils.mysql_utils import MysqlUtils


class EAppMysqlInstanceRestore(MysqlInstanceRestore):
    def __init__(self, p_id, job_id, sub_job_id, json_param):
        super().__init__(p_id, job_id, sub_job_id, json_param)
        self._restore_step = 0
        self._error_code = 0

    def call_xtrabackup(self, restore_type, copy_path, mysql_data_dir):
        log_pattern = MysqlUtils.find_bin_log_pattern(self.my_cnf_path)
        backup_bin_log_files = self.backup_bin_log_files(copy_path, log_pattern)
        ret = super().call_xtrabackup(restore_type, copy_path, mysql_data_dir)
        if not ret:
            return ret
        restore_log_files = copy_path
        if not is_clone_file_system(self._json_param):
            restore_log_files = backup_bin_log_files
        need_restore_logs = []
        for file_name in os.listdir(restore_log_files):
            if file_name.startswith(log_pattern) and file_name != f'{log_pattern}.index':
                need_restore_logs.append(os.path.join(restore_log_files, file_name))
        if not need_restore_logs:
            log.error("No bin log need to restore")
            return False
        log_bin_dir = self.get_restore_binlog_dir()
        ret = mysql_backup_files(self._sub_job_id, need_restore_logs, log_bin_dir)
        if not ret:
            log.error(f"Restore log file failed. pid:%s jobId:%s, subJobId:%s",
                      self._p_id, self._job_id, self._sub_job_id)
            return False
        log.info("Restore log file success")
        return ret

    def backup_bin_log_files(self, copy_path, log_pattern):
        if is_clone_file_system(self._json_param):
            return ""
        backup_bin_log_files = os.path.join("/tmp", self._sub_job_id)
        exec_mkdir_cmd(backup_bin_log_files, is_check_white_list=False)
        log.info(f"backup_bin_log_files:{backup_bin_log_files}")
        for file_name in os.listdir(copy_path):
            if file_name.startswith(log_pattern):
                exec_mv_cmd(os.path.join(copy_path, file_name), backup_bin_log_files, check_white_black_list_flag=False)
        return backup_bin_log_files

    def allow_restore(self):
        if MysqlUtils.eapp_is_running():
            self._error_code = MySQLErrorCode.CHECK_MYSQL_NOT_CLOSE
            log.error("EAppMySQL is running")
            return False
        return True

    def exec_sub_job_for_eapp(self, restore_type):
        if not self.check_mysql_cluster_is_stop():
            log.error("Cannot restore, eappmysql is running. jobId:%s", self._job_id)
            return False
        log.info(f"Instance restore begin. pid:%s jobId:%s, subJobId:%s",
                 self._p_id, self._job_id, self._sub_job_id)
        self._restore_step = self.read_restore_step_info()
        ret, copy_path = self.get_restore_copy_path()
        log.info(f"copy_path before: {copy_path}")
        if not is_clone_file_system(self.get_json_param()):
            data_tmp_dir = os.path.join("/tmp", self._job_id)
            exec_cp_cmd(copy_path, data_tmp_dir, is_check_white_list=False)
            copy_path = data_tmp_dir
        log.info(f"copy_path after: {copy_path}")
        if not ret:
            log.error(f"Exec set restore all param failed. pid:%s jobId:%s, subJobId:%s",
                      self._p_id, self._job_id, self._sub_job_id)
            return False
        progress_type = self._sub_job_id if self._sub_job_id else self._job_id
        self.write_progress_file(SubJobStatusEnum.RUNNING.value, 1, progress_type)
        ret, mysql_data_dir = self.get_mysql_storage_path()
        if not ret:
            return False
        # 此处可能存在mysql运行中的进程，直接kill进程
        # 恢复前，强行停止进程
        MysqlUtils.kill_mysql_process()
        if not self.prepare_restore(restore_type, mysql_data_dir, copy_path):
            return False
        if not self.call_xtrabackup(restore_type, copy_path, mysql_data_dir):
            return False
        if not self.operate_file_power_after_restore(mysql_data_dir):
            return False
        log.info(f"Restore sub job success. pid:%s jobId:%s, subJobId:%s",
                 self._p_id, self._job_id, self._sub_job_id)
        return True

    def restart_cluster_for_eapp(self, restore_type):
        if not self.check_mysql_cluster_is_stop():
            return False
        ret, mysql_data_dir = self.get_mysql_storage_path()
        if not ret:
            log.error("Failed to get mysql data dir")
            return False
        if not self.operate_chown(mysql_data_dir):
            log.error("Failed to change owner for data dir")
            return False
        cache_path = self.get_mount_path(RepositoryDataTypeEnum.CACHE_REPOSITORY.value)
        target_nodes = self.get_json_param().get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.TARGETENV, {}).get(
            MySQLJsonConstant.NODES, [])
        ret, server_id = MysqlUtils.generate_server_id(cache_path, len_nodes=len(target_nodes),
                                                       my_cnf_path=self.my_cnf_path)
        if not ret:
            log.error("Failed to generate server-id")
            return False
        if not MysqlUtils.save_server_id(server_id, self.my_cnf_path):
            log.error("Failed to save server-id")
            return False
        if not MysqlUtils.enable_skip_slave_start(self.is_new_location(), self.my_cnf_path):
            log.error("Failed to enable skip-slave-start")
            return False
        self.restart_cluster(restore_type)
        running = MysqlUtils.eapp_is_running()
        times = 0
        while times < 5 and not running:
            self.restart_cluster(restore_type)
            running = MysqlUtils.eapp_is_running()
            if not running:
                log.warn("Failed to restart cluster, wait retry.times:%s", times)
                time.sleep(20)
            times = times + 1
        # 多次重启失败后，再次尝试start服务
        if not running and not self.start_mysql(restore_type):
            MysqlUtils.disable_skip_slave_start(self.is_new_location(), self.my_cnf_path)
            log.error("Failed to restart cluster")
        return running

    def start_mysql(self, restore_type):
        # 尝试stop+start进行启动
        execute_cmd("systemctl stop eappmysql")
        ret, _, output = execute_cmd("systemctl start eappmysql")
        if ret != ExecCmdResult.SUCCESS:
            log.error(f"Exec systemctl start eappmysql failed. pid:%s jobId:%s", self._p_id, self._job_id)
            return False
        # 再次重启一次
        self.restart_cluster(restore_type)
        return MysqlUtils.eapp_is_running()

    def stop_slave_for_eapp(self):
        if not self.reset_slave_all():
            log.error("Failed to stop slave")
            return False
        ret, channels = self.change_master()
        if not ret:
            log.error("Failed to change master")
            return False
        if not self.start_slave_for_eapp(channels):
            log.error("Failed to start slave")
            return False
        if not MysqlUtils.disable_skip_slave_start(self.is_new_location(), self.my_cnf_path):
            log.error("Failed to disable skip-slave-start")
            return False
        self.write_restore_step_info(MySQLRestoreStep.STOP_SLAVE)
        log.info("Restart slave success")
        return True

    def reset_slave_all(self):
        retry_nums = 0
        ret = False
        output = ""
        while retry_nums < 3:
            retry_nums += 1
            log.info("Retry time: %s", retry_nums)
            if not self.exec_stop_slave():
                log.error("Failed to stop slave")
                ret = False
                continue
            try:
                exec_sql_param = self.generate_exec_sql_param()
                exec_sql_param.sql_str = "reset slave all"
                ret, output = exec_sql(exec_sql_param)
            except Exception as exception_str:
                output = str(exception_str)
                ret = False
                continue
            if ret:
                break
        if not ret:
            log.error(f"Failed to reset slave all ret:%s, output:%s pid:%s jobId:%s",
                      ret, output, self._p_id, self._job_id)
            return False
        log.info("Reset slave success")
        return True

    def start_slave_for_eapp(self, channels):
        for channel in channels:
            if not self.start_slave(channel):
                log.error("Failed to start slave for channel(%s)", channel)
                return False
        log.info("Start slave success")
        return True

    def get_restore_copy_path(self):
        ret, copy_path = super().get_restore_copy_path()
        if not ret:
            return False, ""
        ret, host_id = self.get_recover_host_id()
        if not ret:
            return False, ""
        return True, os.path.join(copy_path, host_id)

    def get_recover_host_id(self):
        source_nodes = self.get_source_node_ids()
        host_id = self.get_host_sn()
        target_nodes = self.get_target_node_ids()
        if len(source_nodes) != len(target_nodes):
            self._error_code = MySQLErrorCode.ERROR_DIFFERENT_TOPO
            log.error("Not allow restore to different cluster, source:%s, target:%s",
                      len(source_nodes), len(target_nodes))
            return False, ''
        return True, source_nodes[target_nodes.index(host_id)]

    def get_target_node_ids(self):
        target_nodes = self.get_json_param().get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.TARGETENV, {}).get(
            MySQLJsonConstant.NODES, [])
        target_node_ids = [node.get('id', '') for node in target_nodes]
        target_node_ids.sort()
        return target_node_ids

    def get_source_nodes(self):
        copies = self.get_json_param().get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.COPIES, [])
        if not copies:
            log.error("No copies")
            return []
        return copies[-1].get(MySQLJsonConstant.APPENV, {}).get(MySQLJsonConstant.NODES, [])

    def get_source_node_ids(self):
        copy_nodes = self.get_source_nodes()
        copy_node_ids = [node.get('id', '') for node in copy_nodes]
        copy_node_ids.sort()
        return copy_node_ids

    def get_source_idx(self, target_host):
        target_nodes = self.get_json_param().get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.TARGETENV, {}).get(
            MySQLJsonConstant.NODES, [])
        target_id = ''
        for target_node in target_nodes:
            agent_ip_list = target_node.get(MySQLJsonConstant.EXTENDINFO, {}) \
                .get(MySQLJsonConstant.AGENT_IP_LIST, '').split(',')
            if target_host in agent_ip_list:
                target_id = target_node.get(MySQLJsonConstant.ID)
                break
        if not target_id:
            log.error("Error target id:%s", target_host)
            return -1
        target_node_ids = self.get_target_node_ids()
        target_idx = target_node_ids.index(target_id)
        source_node_ids = self.get_source_node_ids()
        source_id = source_node_ids[target_idx]
        source_nodes = self.get_source_nodes()
        for idx, source_node in enumerate(source_nodes):
            if source_id == source_node.get(MySQLJsonConstant.ID):
                return idx
        return -1

    def get_source_auth_info(self, target_host):
        idx = self.get_source_idx(target_host)
        if idx == -1:
            return '', ''
        user_key = f"job_copies_0_protectEnv_nodes_{idx}_auth_authKey_{self._p_id}"
        pwd_key = f"job_copies_0_protectEnv_nodes_{idx}_auth_authPwd_{self._p_id}"
        return user_key, pwd_key

    def get_master_node_info(self, nodes):
        master_info_list = []
        target_host_id = self.get_host_sn()
        correct_ips = []
        for node in nodes:
            correct_ips.extend(node.get(MySQLJsonConstant.EXTENDINFO, {})
                               .get(MySQLJsonConstant.AGENT_IP_LIST, '').split(','))
        for node in nodes:
            if node.get('id') == target_host_id:
                master_info_str = node.get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.MASTER_INFO)
                if not master_info_str:
                    log.error("No master info")
                    return False, []
                master_list = json.loads(master_info_str)
                for master in master_list:
                    master_host = master['host']
                    if master_host not in correct_ips:
                        continue
                    mysql_user_key, mysql_pwd_key = self.get_source_auth_info(master_host)
                    if not mysql_user_key or not mysql_pwd_key:
                        log.error("No found source node")
                        return False, []
                    user = safe_get_environ(mysql_user_key)
                    mysql_pwd = safe_get_environ(mysql_pwd_key)
                    master_info_list.append(MasterInfoParseParam(master_host, user, mysql_pwd, master['port']))
                break
        return True, master_info_list

    def change_master(self):
        nodes = self.get_json_param().get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.TARGETENV, {}).get(
            MySQLJsonConstant.NODES, [])
        ret, master_info_list = self.get_master_node_info(nodes)
        if not ret or not master_info_list:
            log.error("Failed to get master info")
            return False, []
        host_id = self.get_host_sn()
        channels = []
        for node in nodes:
            node_id = node.get(MySQLJsonConstant.ID, "")
            if host_id == node_id:
                continue
            master_host = node.get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.INSTANCEIP, '')
            master_info = MysqlUtils.get_master_info_by_host(master_info_list, master_host)
            if not master_info:
                log.error("Failed to get master info for %s", master_host)
                return False, []
            cmd_str = cmd_format("change master to master_host='{}',  \
                      master_port={},  \
                      master_user='{}',  \
                      master_password='{}',  \
                      MASTER_AUTO_POSITION = 1 for channel '{}'",
                                 master_host, master_info.master_port, master_info.master_user,
                                 master_info.master_password, master_host)
            exec_sql_param = self.generate_exec_sql_param()
            exec_sql_param.sql_str = cmd_str
            ret, output = exec_sql(exec_sql_param)
            if not ret:
                log.error(f"Exec_sql failed. sql:change master. ret:{ret} pid:{self._p_id} jobId{self._job_id}")
                return False, []
            channels.append(master_host)
        log.info("Change master success.")
        return True, channels

    def is_new_location(self):
        nodes = self.get_json_param().get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.TARGETENV, {}).get(
            MySQLJsonConstant.NODES, [])
        node_ids = [node.get(MySQLJsonConstant.ID, "") for node in nodes]
        host_id = self.get_host_sn()
        return node_ids.count(host_id) == 0

    def reset_master(self):
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = "reset master"
        ret, output = exec_sql(exec_sql_param)
        if not ret:
            log.error("Failed to reset master, error:%s", output)
            return False
        return True

    def get_gtid_executed(self):
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = "select @@global.gtid_executed"
        ret, output = exec_sql(exec_sql_param)
        if not ret or not output:
            log.error("Failed to get gtid_executed, error:%s", output)
            return ''
        return output[0][0].replace('\n', '')

    def check_sync_status(self):
        ret, error = self.check_cluster_sync_status()
        if ret or "Got fatal error 1236" not in error:
            return True
        executed_infos = self.get_executed_info()
        if not executed_infos:
            log.info("No executed gtid")
            return True
        host_id = self.get_host_sn()
        for master_id, gtid_executed in executed_infos.items():
            gtids = gtid_executed.split(',')
            gap_gtids = MysqlUtils.get_gap_gtids(gtids)
            if not gap_gtids:
                continue
            master_dir = os.path.join(self._cache_path, master_id + MySQLStrConstant.GAP_GTID)
            if not os.path.exists(master_dir) and not exec_mkdir_cmd(master_dir):
                log.error(f"create file[{master_dir}] failed.")
                return False
            gap_file = os.path.join(master_dir, host_id)
            MysqlUtils.save_gap_gtid(gap_file, gap_gtids)
            log.info("Find gap gtid success. master_id:%s, gap gtid:%s", master_id, gap_gtids)
        log.info("Check sync status finished")
        return True

    def compare_master(self):
        master_dir = os.path.join(self._cache_path, self.get_host_sn() + MySQLStrConstant.GAP_GTID)
        if not os.path.exists(master_dir) or not os.path.isdir(master_dir):
            return True
        node_ids = self.get_node_map().keys()
        gtid_purged = self.get_purged_info()
        for gap_file_name in os.listdir(master_dir):
            if gap_file_name not in node_ids:
                continue
            gap_file_path = os.path.join(master_dir, gap_file_name)
            if os.path.isdir(gap_file_path):
                log.warn("Invalid gap file path")
                continue
            lines = MysqlUtils.read_file(gap_file_path)
            new_lines = list()
            for line in lines:
                new_line = MysqlUtils.filter_gap_gtid(gtid_purged, line)
                if new_line.strip():
                    new_lines.append(new_line + '\n')
            if not new_lines:
                continue
            MysqlUtils.save_file(gap_file_path, new_lines)
        log.info("Compare master success")
        return True

    def reset_sync(self):
        node_ids = self.get_node_map().keys()
        need_purge_gtids = list()
        for node_id in node_ids:
            master_dir = os.path.join(self._cache_path, node_id + MySQLStrConstant.GAP_GTID)
            if not os.path.exists(master_dir) or not os.path.isdir(master_dir):
                continue
            need_purge_gtids.extend(MysqlUtils.get_need_purge_gtid(master_dir, node_ids))
        log.info(f"Need purge:{','.join(need_purge_gtids)}")
        if not self.reset_purged_info(need_purge_gtids):
            log.info("Failed to reset sync info")
            return False
        log.info("Reset sync info success")
        return True

    def reset_purged_info(self, need_purge_gtids):
        if not need_purge_gtids:
            log.info("No need to reset sync info.")
            return True
        if not self.reset_slave_all():
            log.error("Failed to reset slave all")
            return False
        need_purge_gtid = ','.join(need_purge_gtids)
        purged_info = self.get_gtid_executed()
        purged_info = f"{purged_info},{need_purge_gtid}"
        self.reset_master()
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = f"set @@global.gtid_purged='{purged_info}'"
        ret, output = exec_sql(exec_sql_param)
        if not ret:
            log.error("Failed to set gtid_purged.error:%s", output)
            return False
        if not self.change_master():
            log.error("Failed to change master")
            return False
        if not self.start_slave():
            log.error("Failed to start slave")
            return False
        log.info('Reset gtid_purged success')
        return True

    def get_executed_info(self):
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = "show slave status"
        ret, output = exec_sql(exec_sql_param)
        if not ret:
            log.error(f"Exec sql failed. sql:show salve status \
                            ret:{ret}  pid:{self._p_id} jobId{self._job_id}")
            return dict()
        executed_infos = dict()
        node_map = self.get_node_map()
        for line in output:
            master_host = line[1]
            master_id = ''
            for node_id, ip_list in node_map.items():
                if master_host in ip_list:
                    master_id = node_id
            executed_infos[master_id] = line[-5]
        return executed_infos

    def get_purged_info(self):
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = "select @@global.gtid_purged"
        ret, output = exec_sql(exec_sql_param)
        if not ret or not output:
            log.error("Failed to get gtid_purged, error:%s", output)
            return ''
        return output[0][0].replace('\n', '')

    def get_node_map(self):
        target_nodes = self.get_json_param().get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.TARGETENV, {}).get(
            MySQLJsonConstant.NODES, [])
        node_map = {}
        for target_node in target_nodes:
            agent_ip_list = target_node.get(MySQLJsonConstant.EXTENDINFO, {}) \
                .get(MySQLJsonConstant.AGENT_IP_LIST, '').split(',')
            node_map[target_node.get(MySQLJsonConstant.ID, '')] = agent_ip_list
        return node_map
