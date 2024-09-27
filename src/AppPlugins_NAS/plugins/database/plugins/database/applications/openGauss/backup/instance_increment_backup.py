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

from common.const import DeployType
from openGauss.backup.instance_backup import InstanceBackup
from openGauss.common.const import Status, ResultCode, SyncMode,\
    MetaDataKey, ProtectSubObject, CopyInfoKey, NodeRole, SUCCESS_RET
from openGauss.common.error_code import BodyErr
from openGauss.common.common import get_value_from_dict, str_to_int, get_previous_copy_info, execute_cmd_by_user
from common.common import convert_time_to_timestamp


class InstanceIncrementBackup(InstanceBackup):
    def __init__(self, pid, job_id, param_json):
        super(InstanceIncrementBackup, self).__init__(pid, job_id, param_json)

    def check_backup_type(self):
        self.log.info(f"Check backup type. job id: {self._job_id}")
        endpoint = self._resource_info.get_local_endpoint()
        if self._resource_info.get_node_status(endpoint) != Status.NORMAL:
            self.log.error(f"Node status is abnormal job id: {self._job_id}")
            return BodyErr.ERR_PLUGIN_CANNOT_BACKUP
        previous_copy = get_previous_copy_info(self._protect_obj, self._job_id)
        ret, last_copy_id = get_value_from_dict(previous_copy, MetaDataKey.EXTEND_INFO, MetaDataKey.BACKUP_INDEX_ID)
        if not isinstance(last_copy_id, str) or not self.get_copy_info_by_copy_id(last_copy_id):
            self.log.error(f"Check exist complete copy failed. job id: {self._job_id}")
            return BodyErr.ERR_INC_TO_FULL
        ret, error_code = self.lsn_check(last_copy_id)
        if not ret:
            self.log.info(f"Present_lsn less than copy_start_lsn! job id: {self._job_id}")
            return error_code
        if self._resource_info.get_deploy_type() == DeployType.SINGLE_TYPE or\
                self._resource_info.get_sync_state() == SyncMode.SYNC:
            self.log.info(f"Single-node cluster or synchronous data replication mode. job id: {self._job_id}")
            return SUCCESS_RET
        _, last_endpoint = get_value_from_dict(previous_copy, MetaDataKey.EXTEND_INFO, MetaDataKey.ENDPOINT)
        if last_endpoint != endpoint:
            self.log.error(f'Present endpoint is not last endpoint. job id: {self._job_id}')
            return BodyErr.ERR_INC_TO_FULL
        self.log.info(f"Check backup type success. job id: {self._job_id}")
        return SUCCESS_RET

    def backup(self):
        """
        执行备份任务
        :return:
        """
        self.log.info(f"Execute instance increment backup. job id: {self._job_id}")
        if not self.pre_backup():
            self.log.error(f"Backup prepose failed. job id: {self._job_id}")
            return False
        return self.exec_instance_backup("ptrack")

    def get_copy_meta_data(self, copy_time):
        self.log.info(f"Get copy meta data!. job id: {self._job_id}")
        ret, copy = self.get_base_meta_data()
        if not ret:
            self.log.error(f"Get base meta failed!. job id: {self._job_id}")
            copy = {}
            return copy
        protect_obj = copy.setdefault(MetaDataKey.PROTECT_OBJECT, {})
        protect_obj[MetaDataKey.SUB_TYPE] = ProtectSubObject.INSTANCE
        copy[MetaDataKey.PG_PROBACKUP_CONF] = self.get_pg_probackup_conf_data()
        copy[MetaDataKey.ENABLE_CBM_TRACKING] = self.get_enable_cbm_tracking_status()
        copy[MetaDataKey.BACKUP_TIME] = convert_time_to_timestamp(copy_time)
        parent_id_list = self.get_dependency_copy_chain()
        copy[MetaDataKey.PARENT_COPY_ID] = parent_id_list
        self.log.info(f"Get copy meta data success!. job id: {self._job_id}")
        return copy

    def get_dependency_copy_chain(self):
        self.log.info(f"Get dependency copy chain. job id: {self._job_id}")
        previous_copy = get_previous_copy_info(self._protect_obj, self._job_id)
        if not previous_copy:
            self.log.error(f"Get previous copy failed!. job id: {self._job_id}")
            return []
        ret, parent_id_list = get_value_from_dict(previous_copy, MetaDataKey.EXTEND_INFO, MetaDataKey.PARENT_COPY_ID)
        if not ret:
            self.log.error(f"Get parent id list failed!. job id: {self._job_id}")
            return []
        self.log.info(f"Get parent id list success!. job id: {self._job_id}")
        ret, parent_backup_key = get_value_from_dict(previous_copy,
                                                     MetaDataKey.EXTEND_INFO, MetaDataKey.BACKUP_INDEX_ID)
        parent_id_list.append(parent_backup_key)
        return parent_id_list

    def get_present_lsn(self):
        self.log.info(f"Get present lsn. job id: {self._job_id}")
        endpoint = self._resource_info.get_local_endpoint()
        if self._resource_info.get_deploy_type() == DeployType.SINGLE_TYPE or\
                self._resource_info.get_node_role(endpoint) == NodeRole.PRIMARY:
            ret = self.get_primary_node_lsn()
            return ret
        ret = self._resource_info.get_node_receiver_replay_location(endpoint)
        return ret

    def get_primary_node_lsn(self):
        self.log.info(f"Get primary node lsn. job id: {self._job_id}")
        cmd = f'gsql -c \"select pg_current_xlog_location();\" postgres -p {self._port}'
        ret, std_out, std_err = execute_cmd_by_user(self._user_name, self._env_file, cmd)
        if ret != ResultCode.SUCCESS:
            self.log.error(f'Get present lsn failed. job id: {self._job_id}')
            return ""
        out_list = std_out.split("\n")
        if len(out_list) < 3:
            self.log.error(f'Bad std_out. job id: {self._job_id}')
            return ""
        self.log.info(f"Get primary node lsn. job id: {self._job_id}")
        return out_list[2].strip(" ")

    def get_copy_start_lsn(self, copy_id):
        self.log.info(f"Get last copy start lsn. job id: {self._job_id}")
        copy_info = self.get_copy_info_by_copy_id(copy_id)
        if not copy_info:
            self.log.error(f"Get copy info failed!. job id: {self._job_id}")
            return ""
        ret, copy_backups = get_value_from_dict(copy_info, CopyInfoKey.BACKUPS)
        if not ret or len(copy_backups) <= 0:
            self.log.error(f"Get backups failed. job id: {self._job_id}")
            return ""
        ret, start_lsn = get_value_from_dict(copy_backups[0], CopyInfoKey.START_LSN)
        if not ret:
            self.log.error(f"Get last copy start Lsn failed. job id: {self._job_id}")
            return ""
        self.log.info(f"Get last copy start lsn success. job id: {self._job_id}")
        return start_lsn

    def lsn_check(self, last_copy_id):
        self.log.info(f"Lsn check. job id: {self._job_id}")
        present_lsn = self.get_present_lsn()
        copy_start_lsn = self.get_copy_start_lsn(last_copy_id)
        if not present_lsn or not copy_start_lsn:
            self.log.error(f"Get present_lsn or copy_start_lsn failed! job id: {self._job_id}")
            return False, BodyErr.ERR_PLUGIN_CANNOT_BACKUP
        present_lsn_list = present_lsn.split("/")
        last_lsn_list = copy_start_lsn.split("/")
        if len(present_lsn_list) < 2 or len(last_lsn_list) < 2:
            self.log.error(f"Bad lsn! job id: {self._job_id}")
            return False, BodyErr.ERR_PLUGIN_CANNOT_BACKUP
        present_lsn_head = str_to_int(present_lsn_list[0], 16)
        present_lsn_tail = str_to_int(present_lsn_list[1], 16)
        last_lsn_head = str_to_int(last_lsn_list[0], 16)
        last_lsn_tail = str_to_int(last_lsn_list[1], 16)
        if present_lsn_head >= last_lsn_head and present_lsn_tail >= last_lsn_tail:
            self.log.info(f"Present_lsn bigger than copy_start_lsn! job id: {self._job_id}")
            return True, 0
        self.log.info(f"Present_lsn less than copy_start_lsn! job id: {self._job_id}")
        return False, BodyErr.ERR_INC_TO_FULL