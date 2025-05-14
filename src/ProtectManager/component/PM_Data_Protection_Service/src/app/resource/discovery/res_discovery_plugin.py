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
import json
import time
from datetime import datetime
from threading import Thread, Event

from app.base.db_base import database
from app.common import toolkit
from app.common.clients.scheduler_client import SchedulerClient
from app.common.enums.job_enum import JobLogLevel, JobType, JobStatus
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.event_messages.Discovery.discovery_rest import HostOnlineStatus
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.logger import get_logger
from app.common.toolkit import query_job_list, JobMessage
from app.resource.discovery.plugin_factory import plugin_factory
from app.resource.kafka.topics import RESCAN_ENVIRONMENT
from app.resource.models.resource_models import ResourceTable, EnvironmentTable
from app.resource.schemas.env_schemas import ScanEnvSchema, UpdateEnvSchema
from app.resource.service.common import resource_service, domain_resource_object_service

log = get_logger(__name__)

MIN_SCAN_INTERVAL = 500
DEFAULT_SCAN_INTERVAL = 3600
# 注册任务心跳发送周期
JOB_HEARTBEAT_INTERVAL = 30


def is_exist(params: ScanEnvSchema):
    query_expression = {
        'sub_type': params.sub_type,
        'endpoint': params.endpoint
    }
    exist_env = resource_service.query_environment(
        query_expression)
    if exist_env:
        return True
    return False


def update_schedule_task(params: ScanEnvSchema):
    # 重新提交定时扫描任务
    client = SchedulerClient()
    client.batch_delete_schedules([params.uuid])
    client.submit_interval_job(RESCAN_ENVIRONMENT, job_params=params, job_name=params.uuid,
                               interval=params.rescan_interval_in_sec)
    pass


def delete_schedule_task(schedule_id: str):
    client = SchedulerClient()
    client.batch_delete_schedules([schedule_id])


def add_schedule_task(params: ScanEnvSchema):
    SchedulerClient().submit_interval_job(
        RESCAN_ENVIRONMENT, job_params=params, job_name=params.uuid, interval=params.rescan_interval_in_sec)


class DiscoveryManager:
    """  Environment Discover Manager"""

    def __init__(self, sub_type: ResourceSubTypeEnum):
        self.impl = plugin_factory.create_plugin(sub_type)

    @staticmethod
    def create_job(params: ScanEnvSchema, job_type, message: JobMessage = None):
        request_id = params.job_id
        job_id = toolkit.create_job_center_task(request_id, {
            'requestId': request_id,
            'sourceId': params.uuid,
            'sourceName': params.name,
            'sourceType': params.type,
            'sourceSubType': params.sub_type,
            'sourceLocation': params.endpoint,
            "type": job_type,
            'userId': params.user_id,
            'domainIdList': [params.domain_id],
            'enableStop': False
        }, message)
        if not job_id:
            raise EmeiStorBizException(
                error=CommonErrorCodes.SYSTEM_ERROR, message="scan protected environments error")
        return job_id

    @staticmethod
    def _update_job(request_id: str, job_id: str, update_req: dict):
        toolkit.modify_task_log(request_id, job_id, update_req)

    @staticmethod
    def _close_job(request_id: str, job_id: str, status: JobStatus):
        toolkit.complete_job_center_task(request_id, job_id, {
            "status": status.value,
            "progress": 100
        })

    @staticmethod
    def _build_update_job_log_request(job_id: str, log_label: str, log_info_param="", log_level=JobLogLevel.INFO.value):
        return {
            "jobLogs": [{
                "jobId": job_id,
                "startTime": int(datetime.now().timestamp() * 1000),
                "logInfo": log_label,
                "logInfoParam": [log_info_param] if log_info_param else [],
                "level": log_level
            }]
        }

    @staticmethod
    def heartbeat(request_id, event):
        while not event.is_set():
            toolkit.modify_task_log(request_id, request_id, {
                "progress": -1
            })
            log.info(f"[Register env] heartbeat job {request_id}")
            time.sleep(JOB_HEARTBEAT_INTERVAL)

    @staticmethod
    def _build_job_fail_log_request(job_id: str, log_detail: str, log_detail_param: list):
        time_zone = None
        return {
            "jobLogs": [{
                "jobId": job_id,
                "startTime": int(datetime.now(tz=time_zone).timestamp() * 1000),
                "logInfo": "job_log_environment_scan_label",
                "logInfoParam": ["job_status_fail_label"],
                "logDetail": log_detail,
                "logDetailParam": log_detail_param,
                "level": JobLogLevel.ERROR.value
            }]
        }

    @staticmethod
    def fill_env_param(env_params: ScanEnvSchema, env_res):
        if env_res.cert_name:
            env_params.verify_cert = 1
        if env_params.sub_type in [ResourceSubTypeEnum.vCenter.value, ResourceSubTypeEnum.ESXi.value,
                                   ResourceSubTypeEnum.ESX.value]:
            env_params.sub_type = ResourceSubTypeEnum.VMware.value
        # 扫描会更新定时调度
        env_params.rescan_interval_in_sec = env_res.scan_interval

    @staticmethod
    def check_scan_task_is_running(env_id):
        response = query_job_list({
            'sourceId': env_id,
            'types': JobType.MANUAL_SCAN_RESOURCE.value,
            'statusList': [JobStatus.READY.value, JobStatus.RUNNING.value, JobStatus.ABORTING.value,
                           JobStatus.PENDING.value]
        })
        if response is None:
            log.warn(f"query environment :{env_id} exist scan job failed.")
            return False
        response_info = json.loads(response)
        total_count = response_info.get("totalCount")
        return total_count > 0

    @staticmethod
    def check_scan_task_is_finished(job_id):
        response = query_job_list({
            'jobId': job_id,
            'statusList': [JobStatus.SUCCESS.value, JobStatus.FAIL.value, JobStatus.ABORTED.value,
                           JobStatus.CANCELLED.value, JobStatus.PARTIAL_SUCCESS.value, JobStatus.ABORT_FAILED.value]
        })
        if response is None:
            log.warn(f"query scan job {job_id} failed.")
            return False
        response_info = json.loads(response)
        total_count = response_info.get("totalCount")
        return total_count > 0

    def scan_env(self, params: ScanEnvSchema):
        try:
            if is_exist(params):
                log.info(f'environment is exist, no need discovery params.endpoint={params.endpoint}, \
         params.sub_type={params.sub_type}')
            self.impl.do_scan_env(params, is_rescan=False, is_session_connect=False)
            # 将扫描环境加入定时任务
            add_schedule_task(params)
            return True
        except Exception as ex:
            log.exception(f'Fail external call, Exception - {ex}')
            raise EmeiStorBizException(
                error=CommonErrorCodes.SYSTEM_ERROR, message="scan protected environments error") from ex
        finally:
            pass

    def register_env(self, scan_req: ScanEnvSchema):
        if is_exist(scan_req):
            log.info(f'environment is exist, no need discovery params.endpoint={scan_req.endpoint}, \
        params.sub_type={scan_req.sub_type}')
            return
        result = self.scan_with_job(scan_req, is_rescan=False, is_session_connect=False)
        # 将扫描环境加入定时任务
        if result:
            add_schedule_task(scan_req)

    def do_scan_with_heartbeat(self, params, request_id, is_rescan, is_session_connect):
        event = Event()
        heartbeat_thread = Thread(target=self.heartbeat, args=(request_id, event))
        # 开启心跳线程，定时更新任务的最后更新时间，避免超过30分钟被强制定制
        heartbeat_thread.start()
        # 执行主扫描逻辑
        scan_result = self.impl.do_scan_env(params, is_rescan, is_session_connect)
        if scan_result and scan_result.get("is_exceed_limit"):
            self._update_job(request_id, request_id, self._build_update_job_log_request(
                request_id, "job_log_scan_resource_exceed_limit_label", str(scan_result.get("added_resource_size")),
                JobLogLevel.WARNING.value))
        # 扫描逻辑完成设置时间，用于停止心跳的循环
        event.set()
        while heartbeat_thread.is_alive():
            # 等待心跳线程停止
            time.sleep(10)

    def refresh_env(self, params: ScanEnvSchema):
        if self.check_scan_task_is_running(params.uuid):
            log.info("manual scan task is running. refresh env cancelled.")
            return
        # 当数据里状态处于非在线离线时不执行刷新逻辑
        with database.session() as session:
            env = session.query(EnvironmentTable).filter(EnvironmentTable.uuid == params.uuid).one_or_none()
            if not env:
                log.error(f"refresh_env failed. environment {params.uuid} is not exist.")
                return
            if env.link_status not in [str(HostOnlineStatus.ON_LINE.value), str(HostOnlineStatus.OFF_LINE.value)]:
                log.error(f"refresh_env failed. environment current state {env.link_status} cannot support.")
                return
        self.impl.do_scan_env(params, is_rescan=True, is_session_connect=False)

    def update_agent_version_and_timestamp(self, params):
        # 升级后版本发生变化，更新版本信息。
        host = params.extend_context.get("host", {})
        if host.get("link_status") == str(HostOnlineStatus.ON_LINE.value):
            with database.session() as session:
                session.query(ResourceTable).filter(ResourceTable.uuid == params.uuid).update(
                    {"version": host.get("version", ""),
                     "created_time": host.get("created_time", "")})
        else:
            log.info(f"host link status of {self} is not online")

    def modify_env(self, params: UpdateEnvSchema):
        try:
            result = self.impl.do_modify_env(params)
            if result:
                update_schedule_task(result)
        except Exception as ex:
            log.error(f'Fail external call, Exception - {ex}')
            if isinstance(ex, EmeiStorBizException):
                raise ex
            else:
                raise EmeiStorBizException(
                    error=CommonErrorCodes.SYSTEM_ERROR,
                    message="refresh protected environments meet exception") from ex
        finally:
            # agent主机从不存在版本升级到存在版本时。
            self.update_agent_version_and_timestamp(params)

    def delete_env(self, params: str):
        try:
            self.impl.do_delete_env(params)
            delete_schedule_task(params)
        except Exception as ex:
            log.error(f'Fail external call, Exception - {ex}')
            if isinstance(ex, EmeiStorBizException):
                raise ex
            else:
                raise EmeiStorBizException(
                    error=CommonErrorCodes.SYSTEM_ERROR, message="delete protected environments meet exception") from ex
        finally:
            pass

    def pre_check(self, params: ScanEnvSchema):
        self.impl.pre_check(params)

    def manual_scan_env(self, env_id: str, job_id: str):
        try:
            env_param = self.build_manual_scan_param(env_id, job_id)
            self.scan_with_job(env_param, is_rescan=True, is_session_connect=False)
        except Exception as ex:
            log.exception(f"scan env error. job id:{job_id}, error: {ex}")
            self.resolve_exception_job_log(ex, job_id)

    def scan_with_job(self, scan_req: ScanEnvSchema, is_rescan: bool, is_session_connect: bool):
        request_id = scan_req.job_id
        try:
            log.info(f"[Scan env] ,ip={scan_req.endpoint} start")
            # 任务状态更新为运行中
            self._update_job(request_id, request_id, {
                "status": JobStatus.RUNNING.value
            })
            self._update_job(request_id, request_id, self._build_update_job_log_request(
                request_id, "job_scan_env_start_label"))
            # 开始扫描
            self.do_scan_with_heartbeat(scan_req, request_id, is_rescan, is_session_connect)
            self._update_job(request_id, request_id, self._build_update_job_log_request(
                request_id, "job_scan_env_finish_label"))
            # 关闭资源扫描任务
            self._close_job(request_id, request_id, JobStatus.SUCCESS)
            return True
        except Exception as ex:
            log.exception(f"[Scan env] Scan env error. job id:{request_id}, error: {ex}")
            self.resolve_exception_job_log(ex, request_id)
            return False

    def resolve_exception_job_log(self, ex, request_id):
        if isinstance(ex, EmeiStorBizException):
            self._update_job(request_id, request_id, self._build_job_fail_log_request(
                request_id, ex.error_code, ex.parameter_list))
        else:
            self._update_job(request_id, request_id, self._build_job_fail_log_request(
                request_id, CommonErrorCodes.SYSTEM_ERROR.get("code"), []))
        self._close_job(request_id, request_id, JobStatus.FAIL)

    def build_manual_scan_param(self, env_id: str, job_id):
        env_res = resource_service.query_resource_by_id(env_id)
        if env_res is None:
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST, parameters=[])
        env_params = ScanEnvSchema.parse_obj(env_res.dict())
        env_params.job_id = job_id
        self.fill_env_param(env_params, env_res)
        return env_params
