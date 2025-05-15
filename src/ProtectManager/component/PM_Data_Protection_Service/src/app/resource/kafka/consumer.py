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
import datetime
from concurrent.futures._base import Future

from app.base.db_base import database
from app.common.clients import job_center_client
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack
from app.protection.object.common.constants import ResourceProtectionJobSteps
from app.protection.object.schemas import protected_object
from app.protection.object.schemas.protected_object import BatchProtectionExecuteReq, ModifyProtectionExecuteReq
from app.protection.object.service.batch_protection_service import BatchProtectionService
from app.resource.common.constants import HostMigrateJobSteps, HostMigrateConstants
from app.resource.discovery.res_discovery_plugin import DiscoveryManager
from app.resource.kafka.topics import RESOURCE_PROTECTION_MODIFY_SCHEDULE, RESCAN_ENVIRONMENT, \
    SCAN_VM_UNDER_COMPUTE_RES, \
    DB_RESOURCE_DESESTITATION, DB_RESTORE_CREATE_RESOURCE, RESOURCE_PROTECTION_SCHEDULE, \
    MIGRATE_HOST_IMMEDIATE_SCHEDULE, MIGRATE_HOST_INTERVAL_SCHEDULE, MIGRATE_HOST_UNEXPECTED_END_SCHEDULE, \
    RESCAN_ENVIRONMENT_GROUP
from app.resource.models.resource_models import ResourceTable
from app.resource.schemas.database_schema import DBRestoreCreateResourceSchema
from app.resource.schemas.db_desesitization_schema import DbDesestitationTaskSchema
from app.resource.schemas.env_schemas import ScanEnvSchema
from app.resource.schemas.host_models import HostMigrationSchedule
from app.resource.service.common import resource_service, db_desesitization_service, db_restore_create_res_service
from app.resource.service.host.host_migrate_service import HostMigrateObjectService
from app.resource.service.host.host_service import update_host_online
from app.resource.service.vmware.service_instance_manager import service_instance_manager
from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService
from app.settings import resource_client
from app.common.concurrency import DEFAULT_ASYNC_POOL
from app.common.enums.job_enum import JobStatus, JobLogLevel
from app.common.enums.resource_enum import ProtectionStatusEnum
from app.common.event_messages.Resource.resource import ResourceAddedRequest, ResourceDeletedRequest
from app.common.logger import get_logger

log = get_logger(__name__)


@exter_attack
@resource_client.topic_handler(topic=RESCAN_ENVIRONMENT, group=RESCAN_ENVIRONMENT_GROUP, auto_offset_reset='latest')
def rescan_environment(request, **params):
    request_id = request.request_id
    log.info(f"rescan environment request: request_id{request_id}")
    scan_env_schema = ScanEnvSchema(**params)
    discovery_manager = DiscoveryManager(scan_env_schema.sub_type)
    # 改为异步消费,避免kafka消息阻塞
    DEFAULT_ASYNC_POOL.submit(discovery_manager.refresh_env, scan_env_schema)


@exter_attack
@resource_client.topic_handler(SCAN_VM_UNDER_COMPUTE_RES)
def rescan_vm_under_compute_resource(request, **params):
    """
    扫描计算资源下的虚拟机
    :param request:
    :return:
    """
    request_id = request.request_id
    resource_id = params.get("resource_id")
    log.info(f"rescan vm under compute resource. request: request_id={request_id},resource_id={resource_id}")
    # 查询主机
    res = resource_service.query_resource_by_id(resource_id)

    root_rs = resource_service.query_environment({"uuid": res.root_uuid})
    service_instance = service_instance_manager.get_service_instance(
        res.root_uuid)
    env_schema = ScanEnvSchema()
    env_schema.__dict__.update(root_rs[0])
    service = VMwareDiscoveryService(service_instance, env_schema)
    service.scan_vm_under_compute_resource(res, res.name)


@exter_attack
@resource_client.topic_handler(ResourceAddedRequest.default_topic)
def resource_added(request, **params):
    request_id = request.request_id
    resource_id = params.get("resource_id")
    log.info(f"request_id: {request_id}, resource_id: {resource_id}")
    if not resource_id:
        return
    BatchProtectionService.sync_resource_add(resource_id=resource_id)
    db_desesitization_service.add_desestitation_info(resource_id)


@exter_attack
@resource_client.topic_handler(ResourceDeletedRequest.default_topic)
def resource_deleted(request, **params):
    request_id = request.request_id
    resource_id = params.get("resource_id")
    log.info(f"request_id: {request_id}, resource_id: {resource_id}")
    if not resource_id:
        return
    with database.session() as session:
        # 资源还在，直接返回
        count = session.query(ResourceTable.uuid).filter(
            ResourceTable.uuid == resource_id).count()
        if count > 0:
            return
        BatchProtectionService.batch_remove_protection(
            session=session, resource_ids=[resource_id])
    db_desesitization_service.clean_desestitation_info(resource_id)


@exter_attack
@resource_client.topic_handler(DB_RESOURCE_DESESTITATION)
def desestitation_db(request, **params):
    db_desestitation_task_schema = DbDesestitationTaskSchema(**params)
    db_desesitization_service.desestitation_db(db_desestitation_task_schema)


@exter_attack
@resource_client.topic_handler(DB_RESTORE_CREATE_RESOURCE)
def db_restore_reate_resource(request, **params):
    log.info(f"[DB_Restore_Create_Resource]:")
    db_restore_create_resource_schema = DBRestoreCreateResourceSchema(**params)
    db_restore_create_res_service.create_res(db_restore_create_resource_schema)


@exter_attack
@resource_client.topic_handler(RESOURCE_PROTECTION_SCHEDULE)
def protect_resource(request, **params):
    request_id = request.request_id
    if not job_center_client.query_is_job_present(request_id):
        return
    res = resource_service.query_resource_by_id(params.get("resources")[0].get("resource_id"))
    if not res:
        log.info(f"protection resource is not exist!job_id: {params.get('job_id', '')}")
        record_protection_failed_step(params.get("job_id", ''), request_id)
        return
    ext_param = protected_object.extend_manager.get_ext_params_class(res.sub_type, params.pop("ext_parameters"))
    create_request = BatchProtectionExecuteReq(ext_parameters=ext_param, request_id=request_id, **params)
    future = DEFAULT_ASYNC_POOL.submit(BatchProtectionService.execute_batch_protection, request_id, create_request)
    future.add_done_callback(lambda f: complete_job_on_exception(f, create_request))


def complete_job_on_exception(future: Future, create_request: BatchProtectionExecuteReq):
    job_id = create_request.job_id
    request_id = create_request.request_id
    exception = future.exception()
    if exception is not None:
        log.exception(f"protect resource failed. request_id: {request_id}, job_id: {job_id}")
        record_protection_failed_step(job_id, request_id)
        resources = list(resource.resource_id for resource in create_request.resources)
        if resources:
            revert_resource_protection_status(resources)
    else:
        log.info(f"protect resource success. request_id: {request_id}, job_id: {job_id}")


def record_protection_failed_step(job_id, request_id):
    BatchProtectionService.record_job_step(job_id, request_id,
                                           ResourceProtectionJobSteps.PROTECTION_FAILED, JobLogLevel.FATAL)
    BatchProtectionService.update_job(job_id, request_id, JobStatus.FAIL, JobLogLevel.FATAL)


def revert_resource_protection_status(resources):
    with database.session() as session:
        session.query(ResourceTable).filter(
            ResourceTable.uuid.in_(resources),
            ResourceTable.protection_status == ProtectionStatusEnum.protecting
        ).update({
            ResourceTable.protection_status: ProtectionStatusEnum.unprotected
        }, synchronize_session='fetch')


@exter_attack
@resource_client.topic_handler(RESOURCE_PROTECTION_MODIFY_SCHEDULE)
def modify_resource_protection(request, **params):
    request_id = request.request_id
    ext_param = protected_object.extend_manager.get_ext_params_class(params.get("resource_sub_type"),
                                                                     params.pop("ext_parameters"))
    modify_request = ModifyProtectionExecuteReq(
        ext_parameters=ext_param,
        request_id=request_id,
        **params)
    DEFAULT_ASYNC_POOL.submit(BatchProtectionService.modify_protection_execute, modify_request)


def complete_host_migrate_job_on_exception(future: Future, create_request: HostMigrationSchedule):
    job_id = create_request.job_id
    request_id = create_request.job_id
    exception = future.exception()
    if exception:
        log.exception(f"host migrate failed. request_id: {request_id}, job_id: {job_id}")

        log_detail = exception.error_code if isinstance(exception, EmeiStorBizException) else None
        log_detail_param = exception.parameter_list if isinstance(exception, EmeiStorBizException) else None

        HostMigrateObjectService.record_job_step(job_id, request_id, HostMigrateJobSteps.HOST_MIGRATE_FAILED,
                                                 JobLogLevel.FATAL, log_detail, log_detail_param)
        HostMigrateObjectService.update_job(job_id, request_id, JobStatus.FAIL, JobLogLevel.FATAL)
        # 清理调度 清理redis
        update_host_online(str(create_request.host_id))
        HostMigrateObjectService.clear_migrate_schedule_and_context(create_request.host_id)

    else:
        log.info(f"host migrate success. request_id: {request_id}, job_id: {job_id}")


@exter_attack
@resource_client.topic_handler(topic=MIGRATE_HOST_IMMEDIATE_SCHEDULE)
def migrate_host_immediate_resource(request, **params):
    # 立即调度任务
    request_id = request.request_id
    log.info(f"[migrate immediate topic] request_id:{request_id}")
    future = DEFAULT_ASYNC_POOL.submit(HostMigrateObjectService.host_migrate_immediate_tasks,
                                       request_id, HostMigrationSchedule(**params))
    future.add_done_callback(lambda f: complete_host_migrate_job_on_exception(f, HostMigrationSchedule(**params)))


@exter_attack
@resource_client.topic_handler(topic=MIGRATE_HOST_INTERVAL_SCHEDULE)
def migrate_host_interval_resource(request, **params):
    # 周期性调度任务
    request_id = request.request_id
    log.info(f"[Migrate interval topic] request_id:{request_id}")
    future = DEFAULT_ASYNC_POOL.submit(HostMigrateObjectService.host_migrate_interval_schedule_task,
                                       HostMigrationSchedule(**params))
    future.add_done_callback(lambda f: complete_host_migrate_job_on_exception(f, HostMigrationSchedule(**params)))


@exter_attack
@resource_client.topic_handler(topic=MIGRATE_HOST_UNEXPECTED_END_SCHEDULE)
def migrate_host_interval_unexpected_resource(request, **params):
    # 异常结束周期性调度任务
    request_id = request.request_id
    log.debug(f"[Migrate unexpected topic] request_id:{request_id}")
    start_time = params.get("time", "")
    if not start_time:
        log.error("[Migrate unexpected topic] time error")
    job_id = params.get("job_id", "")
    host_id = params.get("host_id", "")
    if (datetime.datetime.now() - datetime.datetime.strptime(start_time, "%Y-%m-%d %H:%M:%S")) \
            .total_seconds() >= HostMigrateConstants.TWO_DAY:
        HostMigrateObjectService.clear_migrate_schedule_and_context(host_id)
        # 更新任务为失败
        HostMigrateObjectService.update_job(job_id, job_id, JobStatus.FAIL)
