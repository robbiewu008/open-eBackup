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

from common.const import BackupTypeEnum
from mongodb.comm.const import MongoDBCode, ErrorCode, MongoSubJob
from mongodb.service.backup.instance_backup import MetaBackup


class ShardBackup(MetaBackup):
    def __init__(self, pid, param_obj):
        super().__init__(pid, param_obj)
        self.support_oplog = True
        self.backup_sub_job_maps = {
            BackupTypeEnum.FULL_BACKUP: [
                MongoSubJob.PRE_CHECK, MongoSubJob.STOP_BALANCE, MongoSubJob.SNAPSHOT,
                MongoSubJob.REPORT_COPY_INFO, MongoSubJob.RESUME_BALANCE
            ],
            BackupTypeEnum.LOG_BACKUP: [
                MongoSubJob.PRE_CHECK, MongoSubJob.OPLOG, MongoSubJob.REPORT_COPY_INFO
            ],
            BackupTypeEnum.INCRE_BACKUP: [
                MongoSubJob.PRE_CHECK, MongoSubJob.OPLOG, MongoSubJob.REPORT_COPY_INFO
            ]
        }

    def gen_mongos_sub_job(self, agent_id, sub_jobs):
        gen_jobs = []
        if self.backup_type != BackupTypeEnum.FULL_BACKUP:
            sub_job = self.gen_sub_job_detail(agent_id, "report_copy_info", 2)
            gen_jobs.append(sub_job)
            return gen_jobs

        for ind, job in enumerate(sub_jobs):
            if job not in ("pre_check", "snapshot"):
                sub_job = self.gen_sub_job_detail(agent_id, job, ind)
                gen_jobs.append(sub_job)

        return gen_jobs

    def gen_repel_sub_job(self, agent_id, sub_jobs):
        gen_jobs = []
        for ind, job in enumerate(sub_jobs):
            if job in ("pre_check", "snapshot", "oplog"):
                sub_job = self.gen_sub_job_detail(agent_id, job, ind)
                gen_jobs.append(sub_job)
        return gen_jobs

    def gen_sub_job(self):
        sub_jobs = self.backup_sub_job_maps.get(self.backup_type, [])
        if not sub_jobs:
            return []
        jobs = []
        agent_set = set()
        execute_insts = self.param.get_all_primary_inst()
        for inst in execute_insts:
            agent = inst.get("id")
            cluster_type = inst.get("shardClusterType")
            role = inst.get("role")
            if role != "1":
                continue
            sub_job = []
            if cluster_type == "mongos":
                sub_job = self.gen_mongos_sub_job(agent, sub_jobs)
            elif agent not in agent_set:
                sub_job = self.gen_repel_sub_job(agent, sub_jobs)
            if sub_job:
                agent_set.add(agent)
                jobs.extend(sub_job)
        return jobs

    def backup(self):
        execute_map = {
            "stop_balance": self.stop_balance,
            "snapshot": self.snap_backup,
            "resume_balance": self.resume_balance,
            "oplog": self.dump_backup
        }
        sub_job_name = self.param.get_sub_job_name()
        callable_method = execute_map.get(sub_job_name)
        if not callable_method:
            return MongoDBCode.FAILED.value, "No sub job to execute,check it."
        code, msg = callable_method()
        return code, msg

    def stop_balance(self):
        execute_handlers = self.get_execute_handlers()
        if not execute_handlers:
            return ErrorCode.NO_INSTANCE_RUNNING, "No instance running, stop balance failed."
        ret = False
        for handler in execute_handlers:
            if handler.inst_info.get("shardClusterType") == "mongos":
                ret = handler.stop_balance()
                break
        if not ret:
            return ErrorCode.FAILED_STOP_BALANCE, "Stop balance failed."
        return MongoDBCode.SUCCESS.value, "Stop balance success."

    def record_balance_status(self, status):
        cont = self.read_cache_info()
        cont["balancer"] = status
        self.write_cache_cont(cont)

    def resume_balance(self):
        cont = self.read_cache_info()
        status = cont.get("balancer", True)
        execute_handlers = self.get_execute_handlers()
        if not execute_handlers:
            return ErrorCode.NO_INSTANCE_RUNNING, "No instance running, stop balance failed."
        for handler in execute_handlers:
            if handler.inst_info.get("shardClusterType") == "mongos":
                handler.resume_balance(status)
        return MongoDBCode.SUCCESS.value, "Resume balance success."
