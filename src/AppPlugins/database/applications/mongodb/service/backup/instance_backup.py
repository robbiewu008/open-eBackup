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

import math
import os
import time

from bson import decode_file_iter, CodecOptions

from common.common import clean_dir, read_tmp_json_file
from common.common_models import SubJobModel, CopyInfoRepModel, ReportCopyInfoModel, Copy
from common.const import SubJobTypeEnum, SubJobPolicyEnum, RpcParamKey, BackupTypeEnum
from common.file_common import create_dir_recursive
from common.util.backup import backup, query_progress
from common.util.backup_utils import BackupStatus
from mongodb import LOGGER
from mongodb.comm.cmd import Cmd
from mongodb.comm.const import TMP_MOUNT_PATH, ErrorCode, MongoDBCode, INSTANCE_DIRECTORY, LOG_DIRECTORY, ParamField, \
    MongoSubJob, Status, DefaultValue, REPORT_INTERVAL_SEC, MongoTool, SINGLE_TYPE, MongoRoles
from mongodb.comm.exception_err import DBAuthenticationError, DBConnectionError
from mongodb.comm.mongo_executor import DB
from mongodb.comm.utils import read_file_with_lock, write_file_with_lock, get_previous_copy_info, FileLock, \
    output_result_ignore_exists
from mongodb.service.backup.snapshot import Snap


class MetaBackup:
    def __init__(self, pid, param_obj):
        self.support_oplog = False
        self.lvms = []
        self.snap_lvms = []
        self.dump = None
        self.cmd = Cmd(pid)
        self.snap = Snap(self.cmd)
        self.param = param_obj
        self.backup_type = self.param.get_backup_type()
        self.online_instance = []
        self.offline_instance = []
        self.auth_failed_instance = []
        self.inst_handles = []
        self.default_snap_size = DefaultValue.SNAP_SIZE.value
        self.backup_sub_job_maps = {
            BackupTypeEnum.FULL_BACKUP: [MongoSubJob.PRE_CHECK, MongoSubJob.SNAPSHOT, MongoSubJob.REPORT_COPY_INFO],
            BackupTypeEnum.LOG_BACKUP: [MongoSubJob.PRE_CHECK, MongoSubJob.OPLOG, MongoSubJob.REPORT_COPY_INFO],
            BackupTypeEnum.INCRE_BACKUP: [MongoSubJob.PRE_CHECK, MongoSubJob.OPLOG, MongoSubJob.REPORT_COPY_INFO]
        }
        self._local_instances = self.param.get_local_insts_info()
        self.job_id = self.param.job_id

    @classmethod
    def wait_backup_progress(cls, backup_job_id):
        count = 0
        while count < 10:
            status, progress, size = query_progress(backup_job_id)
            if status == BackupStatus.BACKUP_COMPLETED.value:
                return status, progress, size
            if status == BackupStatus.BACKUP_INPROGRESS.value:
                break
            time.sleep(1)
            count += 1
        while True:
            status, progress, size = query_progress(backup_job_id)
            if status != BackupStatus.BACKUP_INPROGRESS.value:
                return status, progress, size
            time.sleep(REPORT_INTERVAL_SEC)

    def get_local_online_instances(self):
        self._parse_local_instances_status()
        return self.online_instance

    def get_strategy_inst_uris(self):
        return self.param.get_primary_uris()

    def get_execute_handlers(self) -> list:
        if not self.online_instance:
            self._parse_local_instances_status()
        execute_handlers = []
        for handle in self.inst_handles:
            if handle.is_primary():
                execute_handlers.append(handle)
        return execute_handlers

    def get_local_role(self) -> list:
        return self.get_execute_handlers()

    def get_local_insts_status(self):
        self._parse_local_instances_status()
        if not self.online_instance:
            return Status.OFFLINE
        return Status.ONLINE if self.get_execute_handlers() else Status.OFFLINE

    def get_lvms(self):
        """
        获取集群在当前节点所有实例的所在逻辑卷信息
        :return:
        """
        if not self.online_instance:
            self._parse_local_instances_status()
        inst_handles = self.get_execute_handlers()
        # 所有实例的逻辑卷信息
        lvm_map = {}
        for inst in inst_handles:
            if inst.get_inst_type() == "mongos":
                continue
            inst.parse_data_lvm_info(self.snap)
            lvm_name = inst.lvm_name
            fs_type = inst.fs_type
            mount_path = inst.mount_path
            repl_data_path = inst.gen_repl_data_path(mount_path)
            node_id = inst.get_inst_node_id()
            shard_index = inst.get_shard_index()
            inst_info = {
                "repl_data_path": repl_data_path,
                "shardIndex": shard_index
            }
            if lvm_name not in lvm_map:
                lvm_map[lvm_name] = {
                    "mount_path": mount_path,
                    "inst": [inst_info],
                    "id": node_id,
                    "fs_type": fs_type
                }
            else:
                lvm_map[lvm_name]["inst"].append(inst_info)
        # 节点中实例所在逻辑卷添加实例信息
        lvms = []
        lvs = self.snap.show_lvs()
        for lvm_info in lvs:
            lvm_name = lvm_info.get("lv_name")
            if lvm_name in lvm_map:
                lvm_info.update(lvm_map[lvm_name])
                lvms.append(lvm_info)
        self.lvms = lvms

    def check_lvm(self):
        if self.backup_type != BackupTypeEnum.FULL_BACKUP.value:
            return MongoDBCode.SUCCESS.value, "No need check lvm.", []
        snap_per = self.param.get_lvm_percent()
        LOGGER.info(f"Start check param of snap_per: {snap_per}")
        # 没有传值默认10%
        if not snap_per:
            snap_per = DefaultValue.SNAP_MIN_PER.value
        LOGGER.info(f"Start check param of create_lvm_percent: {snap_per}")
        if not (DefaultValue.SNAP_MIN_PER.value <= snap_per <= DefaultValue.SNAP_MAX_PER.value):
            return ErrorCode.FAILED_CREATE_SNAP.value, f"Mongo Data create lvm percent is {snap_per}, " \
                                                       f"not between 10 from 50", []
        self.get_lvms()
        if not self.lvms:
            return ErrorCode.ONLY_SUPPORT_LVM.value, "Mongo Data not on lvm, not support snap backup", []
        for lvm_info in self.lvms:
            lvm_path = lvm_info.get("lv_path")
            mount_path = lvm_info.get("mount_path", "")
            mount_style = self.snap.get_vg_mount_style(lvm_path, mount_path)
            if mount_style not in ("Name", "Default"):
                return ErrorCode.NOT_SUPPORT_MOUNT.value, "Not support lvm mount.", []
            if not self.snap.check_lvm_free_size(lvm_info, snap_per):
                res_detail = str(snap_per) + "%"
                return ErrorCode.OUT_OF_VG_SPACE.value, "The vgs has no space to snap", [res_detail]
        return MongoDBCode.SUCCESS.value, "Check lvm success.", []

    def check_oplog(self):
        if self.backup_type == BackupTypeEnum.FULL_BACKUP.value:
            return MongoDBCode.SUCCESS.value, "No need check oplog"
        if not self.support_oplog:
            return ErrorCode.NOT_SUPPORT_OPLOG.value, "Not support oplog."
        mongodump_bin_dir = self.param.get_mongo_bin_path(ParamField.MOBGODUMP_BIN_PATH.value,
                                                          MongoTool.MONGODUMP.value, self._local_instances[0])
        if not self.cmd.check_mongo_user_and_path(mongodump_bin_dir):
            return ErrorCode.PARAMS_IS_INVALID.value, "Not suitable user."
        ret, _ = self.cmd.check_mongo_tool(mongodump_bin_dir)
        if not ret:
            LOGGER.error("Not find mongodb backup tools in this node, job id: %s", self.job_id)
            return ErrorCode.NOT_SUPPORT_OPLOG.value, "No backup tool in node, not support log backup."
        inst_handlers = self.get_execute_handlers()
        for handler in inst_handlers:
            if handler.get_inst_type() == "mongos":
                continue
            code, msg = handler.check_oplog()
            if code:
                return code, msg
        code, msg = self.check_backup_type()
        if code:
            return code, msg
        return MongoDBCode.SUCCESS.value, ""

    def get_cache_repo(self):
        cache_path = self.param.get_cache_repo()
        if not cache_path:
            return ""
        return cache_path[0]

    def get_data_repo(self):
        data_path = self.param.get_data_path()
        if not data_path:
            return ""
        return data_path[0]

    def get_log_repo(self):
        data_path = self.param.get_log_path()
        if not data_path:
            return ""
        return data_path[0]

    def gen_tmp_mount_path(self, snap_name):
        cache_path = TMP_MOUNT_PATH
        tmp_mount = os.path.join(cache_path, self.param.job_id, snap_name)
        create_dir_recursive(tmp_mount)
        return tmp_mount

    def gen_back_dst_path(self, shard_index):
        """
        生成复制路径
        :param shard_index:
        :return:
        """
        directory = self.gen_back_dir()
        dst_path = os.path.join(directory, shard_index)
        create_dir_recursive(dst_path)
        return dst_path

    def gen_back_dir(self):
        if self.backup_type == BackupTypeEnum.FULL_BACKUP:
            data_repo = self.get_data_repo()
            directory = INSTANCE_DIRECTORY
        else:
            data_repo = self.get_log_repo()
            directory = LOG_DIRECTORY
        dirs = os.path.join(data_repo, directory, self.param.job_id)
        return dirs

    def get_pre_copy_info(self):
        previous_copy = get_previous_copy_info(
            self.param.job_id,
            self.param.protect_object,
            [RpcParamKey.FULL_COPY, RpcParamKey.LOG_COPY],
        )
        return previous_copy

    def check_backup_type(self):
        if self.backup_type == BackupTypeEnum.FULL_BACKUP.value:
            LOGGER.info("Full backup, do nothing, job id: %s", self.job_id)
            return MongoDBCode.SUCCESS.value, "Full backup no need check backup type"
        last_copy = self.get_pre_copy_info()
        LOGGER.info("Check backup type last copy id: %s, job id: %s",
                    last_copy.get("extendInfo", {}).get("copyId", ""), self.job_id)
        if not last_copy:
            return ErrorCode.LOG_TO_FULL_ERR.value, ""
        last_lsn = last_copy.get("extendInfo", {}).get("lastLsn")
        LOGGER.info("Check backup type last lsn: %s, job id: %s", last_lsn, self.job_id)
        if not self.find_lsn(last_lsn):
            LOGGER.warning("Not all backup instance find lsn, need to full backup, job id: %s", self.job_id)
            return ErrorCode.LOG_TO_FULL_ERR.value, ""
        return MongoDBCode.SUCCESS.value, "Check backup type success."

    def find_lsn(self, last_lsn):
        handles = self.get_execute_handlers()
        for handle in handles:
            inst_role = handle.get_inst_type()
            if inst_role == "mongos":
                continue
            shard_index = handle.get_shard_index()
            lsn = last_lsn.get(shard_index)
            LOGGER.info("Last lsn: %s, shard_index: %s, job id: %s", lsn, shard_index, self.job_id)
            if not handle.find_ts(lsn):
                LOGGER.error("No find last lsn, job id: %s", self.job_id)
                return False
        return True

    def get_last_lsn(self):
        if not self.support_oplog:
            return {}
        handles = self.get_execute_handlers()
        all_ts = {
            handle.get_shard_index(): handle.get_tail_ts()
            for handle in handles
            if handle.inst_info.get("shardClusterType") != "mongos"
        }
        return all_ts

    def gen_sub_job(self):
        sub_jobs = self.backup_sub_job_maps.get(self.backup_type, [])
        jobs = []
        inst_handles = self.param.get_local_insts_info()
        inst_handles = {} if not inst_handles else inst_handles[0]
        execute_agent_id = inst_handles.get(ParamField.ID, "")
        if not execute_agent_id:
            return jobs
        priority = 0
        for job_name in sub_jobs:
            sub_job = self.gen_sub_job_detail(execute_agent_id, job_name, priority)
            priority += 1
            jobs.append(sub_job)
        return jobs

    def gen_sub_job_detail(self, execute_agent, job_name, priority):
        sub_job = SubJobModel(jobId=self.param.job_id, subJobId="", jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value,
                              jobName=job_name, jobPriority=priority, policy=SubJobPolicyEnum.FIXED_NODE.value,
                              execNodeId=execute_agent, ignoreFailed=False, jobInfo="").dict(by_alias=True)
        return sub_job

    def backup(self):
        if self.backup_type != BackupTypeEnum.FULL_BACKUP.value:
            code, msg = self.dump_backup()
        else:
            code, msg = self.snap_backup()
        return code, msg

    def prepare_snap_args(self, lvm_info):
        lvm_name = lvm_info.get("lv_name")
        lvm_size = lvm_info.get("lv_size")
        unique = self.param.job_id[-4:]
        snap_name = "_".join((lvm_name, unique))
        lv_path = lvm_info.get("lv_path")
        lvm_size = self.cal_lvm_ten_per_size(lvm_size)
        return lvm_size, snap_name, lv_path

    def write_cache_cont(self, cont):
        cache_file = os.path.join(self.get_cache_repo(), self.param.job_id)
        write_file_with_lock(cache_file, cont)

    def record_lvms_info(self, lvms_info):
        cont = self.read_cache_info()
        if not cont:
            cont = {"lvms": lvms_info}
        else:
            cont.get("lvms", []).extend(lvms_info)
        self.write_cache_cont(cont)

    def snap_backup(self):
        self.get_lvms()
        if not self.lvms:
            return ErrorCode.NO_INSTANCE_RUNNING.value, "instance not online"

        # 先打快照，不做数据拷贝
        for lvm_info in self.lvms:
            code, msg = self.create_snap(lvm_info)
            if code:
                LOGGER.error("Create snap failed with ret code %s, err: %s, job id: %s.", code, msg, self.job_id)
                return code, msg
        # 记录lsn、数据归档
        for lvm_info in self.lvms:
            code, msg = self.archive_data(lvm_info)
            if code:
                LOGGER.error("Archive backup data failed with ret code %s, err: %s, job id: %s", code, msg, self.job_id)
                return code, msg
        # 将备份元信息写入cache 仓
        self.record_lvms_info(self.lvms)
        return MongoDBCode.SUCCESS.value, "Backup success"

    def create_snap(self, lvm_info):
        lvm_size, snap_name, lv_path = self.prepare_snap_args(lvm_info)
        code, msg = self.snap.create_snap_shot(lvm_size, snap_name, lv_path)
        if code:
            LOGGER.error("Backup failed with create snap err: %s, job id: %s.", msg, self.job_id)
            return code, msg
        lvm_info.update({"snap_name": snap_name})
        return MongoDBCode.SUCCESS.value, "Backup success"

    def archive_data(self, lvm_info):
        snap_name = lvm_info.get("snap_name", "")
        snap_lvm = self.snap.get_lvm_info(snap_name)
        if not snap_lvm:
            err_msg = f"Backup failed, lv:%s not find, job id: %s." % (snap_name, self.job_id)
            LOGGER.error(err_msg)
            return ErrorCode.FAILED_COPY_DATA.value, "Backup failed with error snap."
        snap_path = snap_lvm.get("lv_path")
        tmp_mount = self.gen_tmp_mount_path(snap_name)
        ret = self.snap.mount_lvm(snap_path, tmp_mount, lvm_info.get("fs_type", ""))
        if not ret:
            LOGGER.error("Mount snap failed, job id: %s", self.job_id)
            return ErrorCode.ERROR_MOUNT_PATH.value, "Mount snap failed."
        lvm_info.update({"tmp_mount": tmp_mount, "snap_path": snap_path})
        lsn_map = self.get_last_lsn()
        insts = lvm_info.get("inst", [])
        for inst in insts:
            repl_path = inst.get("repl_data_path")
            shard_index = inst.get("shardIndex", "0")
            if shard_index in lsn_map:
                ts = lsn_map.get(shard_index)
                inst["lastLsn"] = (ts.time, ts.inc)
            src = os.path.join(tmp_mount, repl_path)
            dest = self.gen_back_dst_path(shard_index)
            backup_job_id = self.job_id + shard_index
            res = backup(backup_job_id, src, dest)
            LOGGER.debug("Archive data start backup, shard_index: %s, job id: %s.", shard_index, self.job_id)
            if not res:
                LOGGER.error("Archive back up data failed with backup tools response: %s, backup job id: %s", res,
                             backup_job_id)
                return ErrorCode.FAILED_COPY_DATA.value, "Backup failed with cp data to destination"
            status, progress, size = self.wait_backup_progress(backup_job_id)
            if status == BackupStatus.BACKUP_FAILED.value:
                LOGGER.error(f"Archive back up data failed with backup tools return status: {status}.")
                return ErrorCode.FAILED_COPY_DATA.value, "Backup failed with backup tools return failed."
            LOGGER.debug("Archive data completed, shard_index: %s, job id: %s.", shard_index, self.job_id)
        return MongoDBCode.SUCCESS.value, "Backup success with cp data to destination"

    def cal_lvm_ten_per_size(self, lvm_size: str):
        """计算根据卷的使用大小创建快照的大小"""
        lvm_size = lvm_size.strip().strip("m")
        LOGGER.info(f"Total lvm size is:{lvm_size}")
        try:
            lvm_s = float(lvm_size)
        except ValueError:
            LOGGER.error("Exception with lvm size to float, job id: %s.", self.job_id)
            lvm_s = self.default_snap_size
        else:
            snap_per = self.param.get_lvm_percent()
            # 没有传值默认按10%大小创建snap
            if not snap_per:
                snap_per = DefaultValue.SNAP_MIN_PER.value
            lvm_s = math.ceil(lvm_s * snap_per / 100)
        LOGGER.info("Create lvm size is: %s", lvm_s)
        return f"{lvm_s}{DefaultValue.SNAP_UNIT.value}"

    def dump_backup(self):
        """实现日志备份"""
        LOGGER.debug("Begin to dump, job id: %s.", self.job_id)
        self._parse_local_instances_status()
        handlers = self.get_execute_handlers()
        if not self.inst_handles:
            LOGGER.error("No instance running, job id: %s.", self.job_id)
            return ErrorCode.NO_INSTANCE_RUNNING.value, "instance not online"
        last_copy = self.get_pre_copy_info()
        LOGGER.info("Last copy uuid: %s, length of last copy: %s, job id: %s.",
                    last_copy.get("extendInfo", {}).get("copy_id", ""), len(last_copy), self.job_id)
        last_copy_ex = last_copy.get("extendInfo", {})
        last_lsn = last_copy_ex.get("lastLsn", {})
        if not last_lsn:
            LOGGER.error("Not find last lsn in pre copy,it should trans to full backup, job id: %s.", self.job_id)
            return ErrorCode.ERROR_INCREMENT_TO_FULL.value, "Not find lsn."
        associate = last_copy_ex.get("associatedCopies", [])
        associate.append(last_copy_ex.get("copy_id"))
        LOGGER.info("Dump backup associate: %s, job id: %s.", associate, self.job_id)
        for handler in handlers:
            inst_role = handler.get_inst_type()
            if inst_role == "mongos":
                continue
            shard_index = handler.get_shard_index()
            pre_lsn = last_lsn.get(shard_index)
            dst = self.gen_back_dst_path(shard_index)
            LOGGER.debug("Execute dump with args: %s, %s, %s, job id: %s.", shard_index, pre_lsn, dst, last_lsn,
                         self.job_id)
            ret, msg = self._dump_backup(pre_lsn, dst, handler.uri, handler.get_mongodump_bin_path())
            if not ret:
                LOGGER.error("Execute dump backup failed with err: %s, job id: %s.", msg, self.job_id)
                return ErrorCode.FAILED_BACKUP_OPLOG.value, msg
        self.write_cont_to_file(associate, handlers)
        return MongoDBCode.SUCCESS.value, "Backup success"

    def write_cont_to_file(self, associate, handlers):
        cache_file = os.path.join(self.get_cache_repo(), self.param.job_id)
        lock_file = ".".join((cache_file, "lock"))
        with FileLock(lock_file):
            cont = read_tmp_json_file(cache_file)
            LOGGER.debug("Dump backup, write cont to file, read cont: %s, job id: %s.", cont, self.job_id)
            if cont:
                insts = cont.get("inst", [])
            else:
                insts = []
            tmp_inst = {
                "inst": insts,
                "associatedCopies": associate
            }
            for handler in handlers:
                shard_index = handler.get_shard_index()
                tmp_inst["inst"].append({"shardIndex": shard_index})
                LOGGER.debug("Dump backup, write cont to file, shard_index: %s, job id: %s.", shard_index, self.job_id)
            tmp_inst["end_time"] = time.time()
            LOGGER.debug("Dump backup, write cont to file, tmp_inst: %s, job id: %s.", tmp_inst, self.job_id)
            output_result_ignore_exists(cache_file, tmp_inst)

    def read_last_lsn(self, shard_index):
        dest = self.gen_back_dst_path(shard_index)
        target_file = os.path.join(dest, DefaultValue.LOCAL_DB.value, f"{DefaultValue.OPLOG_COLLECTION.value}.bson")
        first_lsn = None
        last_lsn = None
        with open(target_file, "rb") as fd:
            for oplog in decode_file_iter(fd, CodecOptions(unicode_decode_error_handler="ignore")):
                if "ts" in oplog:
                    last_lsn = oplog["ts"]
                if first_lsn is None and last_lsn is not None:
                    first_lsn = last_lsn
        return first_lsn, last_lsn

    def gen_copy_info(self):
        # 备份的元数据
        cont = self.read_cache_info()
        insts = []
        if self.backup_type != BackupTypeEnum.FULL_BACKUP.value:
            data_repo = self.param.get_log_repo()
            directory = LOG_DIRECTORY
            insts = cont.get("inst", [])
        else:
            data_repo = self.param.get_data_repo()
            directory = INSTANCE_DIRECTORY
            for lvm in cont.get("lvms", []):
                insts.extend(lvm.get("inst", []))
        LOGGER.debug("Gen copy info, instances: %s, job id: %s.", insts, self.job_id)
        first_lsns, last_lsns = self.gen_inst_lsn(insts)
        begin_time = end_time = 0
        for _, lsn in first_lsns.items():
            if lsn and (begin_time > lsn[0] or begin_time == 0):
                begin_time = lsn[0]
        for _, lsn in last_lsns.items():
            if lsn and (end_time == 0 or end_time < lsn[0]):
                end_time = lsn[0]
        bak_timestamp = cont.get("end_time", time.time())
        associated = cont.get("associatedCopies", [])
        metadata = {
            "beginTime": begin_time,
            "endTime": end_time,
            "backupTime": bak_timestamp,
            "copy_id": self.param.job_id,
            "firstLsn": first_lsns,
            "lastLsn": last_lsns,
            "associatedCopies": associated,
        }
        data_rep_rsp = self.prepare_data_rep(data_repo, directory)
        copy_info = Copy(repositories=data_rep_rsp, extendInfo=metadata, timestamp=bak_timestamp)
        LOGGER.debug("Gen copy info , the extend info: %s, job id: %s.", metadata, self.job_id)
        return ReportCopyInfoModel(copy=copy_info, jobId=self.job_id).dict(by_alias=True)

    def prepare_data_rep(self, data_repo, directory):
        if self.backup_type == BackupTypeEnum.LOG_BACKUP.value:
            remote_path = os.path.join(
                data_repo.get('remotePath'),
                f"{self.param.job_id}",
                directory,
                f"{self.param.job_id}"
            )
        else:
            remote_path = os.path.join(
                data_repo.get('remotePath'),
                directory,
                f"{self.param.job_id}"
            )
        data_rep_rsp = [
            CopyInfoRepModel(
                id=data_repo.get("id"),
                repositoryType=data_repo.get("repositoryType"),
                isLocal=data_repo.get("isLocal"),
                protocol="NFS",
                remotePath=remote_path,
                remoteHost=data_repo.get("remoteHost"),
                extendInfo={
                    "fsId": data_repo.get('extendInfo', {}).get("fsId")
                }
            ).dict(by_alias=True)
        ]
        return data_rep_rsp

    def gen_inst_lsn(self, insts):
        first_lsns = {}
        last_lsns = {}
        for inst in insts:
            shard_index = inst.get("shardIndex", "0")
            last_lsn = inst.get("lastLsn", ())
            first_lsn = inst.get("firstLsn", ())
            if not last_lsn and self.support_oplog:
                first, last = self.read_last_lsn(shard_index)
                first_lsn = (first.time, first.inc)
                last_lsn = (last.time, last.inc)
            first_lsns[shard_index] = first_lsn if first_lsn else ()
            # last_lsns的样子： {'0': [1689211222, 1], '4': [1689211218, 1]}
            last_lsns[shard_index] = last_lsn if last_lsn else ()
        return first_lsns, last_lsns

    def post_backup(self):
        cont = self.read_cache_info()
        lvms = cont.get("lvms")
        ret = MongoDBCode.SUCCESS.value
        info = ""
        if not lvms:
            return ret, info, []
        local_node_id = self.param.get_local_agent_id()
        lvms = [lvm for lvm in lvms if lvm.get("id") == local_node_id]
        ret = MongoDBCode.SUCCESS.value
        info = ""
        umount_params = []
        release_params = []
        for lvm_info in lvms:
            tmp_mount = lvm_info.get("tmp_mount")
            snap_lvm = lvm_info.get("snap_path")
            umount_params.append(tmp_mount)
            release_params.append(snap_lvm)
        error_param = [''.join(umount_params), ''.join(release_params)]
        for lvm_info in lvms:
            ret, info, param = self.umount_lvm(lvm_info)
            if ret:
                return ret, info, error_param
            ret, info, release_param = self.release_lvm(lvm_info)
            if ret:
                return ret, info, error_param
        try:
            clean_dir(TMP_MOUNT_PATH)
        except Exception as err:
            LOGGER.warning("Clear tmp dir failed,%s, ignore, job id: %s.", str(err), self.job_id)
        return ret, info, error_param

    def umount_lvm(self, lvm_info):
        retry_num = 0
        tmp_mount = lvm_info.get("tmp_mount")
        ret = MongoDBCode.SUCCESS.value
        info = ""
        error_param = tmp_mount
        while retry_num < 3:
            if tmp_mount:
                ret, info, error_param = self.snap.umount_lvm(tmp_mount)
                if ret == MongoDBCode.SUCCESS.value:
                    break
                retry_num += 1
                time.sleep(60)
        return ret, info, error_param

    def release_lvm(self, lvm_info):
        retry_num = 0
        snap_lvm = lvm_info.get("snap_path")
        ret = MongoDBCode.SUCCESS.value
        info = ""
        error_param = snap_lvm
        while retry_num < 3:
            if snap_lvm:
                ret, info, error_param = self.snap.release_lvm(snap_lvm)
                LOGGER.info("release_lvm ret: %s, info: %s, error_param: %s.", ret, info, error_param)
                if ret == MongoDBCode.SUCCESS.value:
                    break
                retry_num += 1
                time.sleep(60)
        return ret, info, error_param

    def read_cache_info(self):
        cache_file = os.path.join(self.get_cache_repo(), self.param.job_id)
        cont = read_file_with_lock(cache_file)
        return cont

    def _parse_local_instances_status(self):
        for inst in self._local_instances:
            uri = inst.get(ParamField.HOSTURL)
            config = self._get_config_map(uri)
            inst_handler = InstanceHandler(self.param.job_id, uri, config, inst)
            try:
                inst_handler.get_inst_status()
            except DBConnectionError:
                self.offline_instance.append(uri)
            except DBAuthenticationError:
                self.auth_failed_instance.append(uri)
            else:
                self.online_instance.append(uri)
                self.inst_handles.append(inst_handler)

    def _get_config_map(self, uri):
        config = {}
        if self.param.get_resource_type() == SINGLE_TYPE:
            db_user = self.cmd.get_db_user()
            pwd = self.cmd.get_db_pwd()
            auth_type = self.cmd.get_db_user_auth_type()
            if db_user and pwd:
                config = {"username": db_user, "password": pwd, "auth_type": auth_type}
            return config
        node_key_map = self.param.get_nodes_key_map()
        _, node_port = uri.split(":")
        node_id = node_key_map.get(node_port)
        db_user = self.cmd.get_db_user(f"nodes_{node_id}_")
        pwd = self.cmd.get_db_pwd(f"nodes_{node_id}_")
        auth_type = self.cmd.get_db_user_auth_type(f"nodes_{node_id}_")
        config = {}
        if db_user and pwd:
            config = {"username": db_user, "password": pwd, "auth_type": auth_type}
        return config

    def _dump_backup(self, pre_lsn: tuple, dst, uri, mongo_bin_dir):
        timestamp, index = pre_lsn
        query = '{\\"ts\\": {\\"$gt\\": {\\"$timestamp\\": {\\"t\\": %s, \\"i\\": %s}}}}' % (timestamp, index)
        ret, info = self.cmd.dump(query, mongo_bin_dir, uri, dst, self._get_node_id(uri))
        return ret, info

    def _get_node_id(self, uri):
        if self.param.get_resource_type() == SINGLE_TYPE:
            return ""
        node_key_map = self.param.get_nodes_key_map()
        _, node_port = uri.split(":")
        node_id = node_key_map.get(node_port)
        return f"nodes_{node_id}_"


class InstanceHandler:
    def __init__(self, job_id, uri, config, inst_info):
        self.uri = uri
        self.config = config
        self.job_id = job_id
        self.lvm_name = ""
        self.fs_type = ""
        self.lvm_size = ""
        self.free_lvm_size = ""
        self.lvm_used_per = ""
        self.mount_path = ""
        self._conn = None
        self.status = ""
        self.inst_info = inst_info

    def get_inst_status(self):
        bin_path = self.inst_info.get('extendInfo').get('binPath')
        mongo = DB(self.uri, self.config, direct_connection=True, bin_path=bin_path)
        try:
            mongo.connect()
        except DBConnectionError as e:
            self.status = Status.OFFLINE
            err_msg = "Connect mongodb client failed with connection err: %s, job id: %s.", (str(e), self.job_id)
            LOGGER.error(err_msg)
            raise e
        except DBAuthenticationError as e:
            LOGGER.error("Connect mongodb client failed with authentication err, job id: %s.", self.job_id)
            raise e
        else:
            self.status = Status.ONLINE
            self._conn = mongo

    def is_primary(self):
        if self._conn is None:
            self.get_inst_status()
        return self._conn.is_primary()

    def get_inst_role(self):
        return self._conn.get_inst_role()

    def get_shard_index(self):
        shard_index = self.inst_info.get("shardIndex")
        if not shard_index:
            shard_index = "0"
        return shard_index

    def get_mongodump_bin_path(self):
        return self.inst_info.get("extendInfo", {}).get(ParamField.MOBGODUMP_BIN_PATH.value, "")

    def get_inst_type(self):
        shard_type = self.inst_info.get("shardClusterType")
        return shard_type if shard_type else ""

    def get_inst_node_id(self):
        node_id = self.inst_info.get("id", "")
        return node_id

    def get_inst_data_path(self):
        return self._conn.get_data_path()

    def parse_data_lvm_info(self, snap_obj: Snap):
        data_path = self.get_inst_data_path()
        if not data_path:
            LOGGER.debug("Process to parse data lvm info, no data_path, job id: %s.", self.job_id)
            return
        lvm_info = snap_obj.parse_df_info(data_path)
        if not lvm_info:
            return
        mapper_path, self.fs_type, _, self.lvm_size, self.free_lvm_size, self.lvm_used_per, self.mount_path = lvm_info
        self.lvm_name = Snap.parse_lvm_name(mapper_path)

    def gen_repl_data_path(self, mount_path):
        data_path = self.get_inst_data_path()
        relpath = os.path.relpath(data_path, mount_path)
        return relpath

    def check_oplog(self):
        if not self._conn:
            try:
                self.get_inst_status()
            except (DBConnectionError, DBAuthenticationError):
                return ErrorCode.NO_INSTANCE_RUNNING.value, "Get inst status failed, not support oplog"
        if self._conn.get_oplog() is None:
            return ErrorCode.NOT_SUPPORT_OPLOG.value, "This inst not support oplog"
        return MongoDBCode.SUCCESS.value, "Check oplog success"

    def show_balance_status(self):
        if not self._conn:
            self.get_inst_status()
        ret = self._conn.check_balance()
        return ret

    def switch_balance_status(self, status):
        if not self._conn:
            self.get_inst_status()
        ret = self._conn.set_balancer(status)
        return ret

    def stop_balance(self):
        if not self._conn:
            self.get_inst_status()
        if not self.show_balance_status():
            return True
        ret = self._conn.stop_balance()
        return ret

    def resume_balance(self, flag):
        if not self._conn:
            self.get_inst_status()
        ret = self._conn.set_balancer(flag)
        return ret

    def find_ts(self, ts):
        """

        :param ts: str "1231234,2";  tuple (1231234,2) ;list [1233123,23]
        :return: bool
        """
        if not ts:
            LOGGER.error("Find ts is None, job id: %s.", self.job_id)
            return False
        if not self._conn:
            self.get_inst_status()
        ret = self._conn.query_oplog_ts(ts)
        LOGGER.debug("Find ts result: %s , job id: %s", ret, self.job_id)
        return ret is not None

    def get_tail_ts(self):
        if not self._conn:
            self.get_inst_status()
        ts = self._conn.get_oplog_tail_ts()
        return ts


class InstanceBackup(MetaBackup):
    def __init__(self, pid, param_obj):
        super().__init__(pid, param_obj)
        single_type = param_obj.param_dict.get("job", "").get("protectEnv", "").get(
            "extendInfo", "").get("singleType", "")
        self.support_oplog = True if single_type == MongoRoles.SINGLE_NODE_REPL else False

    def get_local_online_instances(self):
        if len(self._local_instances) > 1:
            return []
        self._parse_local_instances_status()
        return self.online_instance
