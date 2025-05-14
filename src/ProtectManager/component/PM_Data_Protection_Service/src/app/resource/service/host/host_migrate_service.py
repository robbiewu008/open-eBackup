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
import uuid
from ipaddress import ip_address, IPv4Address, IPv6Address

from app.base.resource_consts import ResourceConstant
from app.base.db_base import database
from app.common import toolkit
from app.common.context.context import Context
from app.common.enums.job_enum import JobType, JobStatus, JobLogLevel
from app.common.clients.scheduler_client import SchedulerClient
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.event_messages.Discovery.discovery_rest import HostOnlineStatus
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException, IllegalParamException
from app.common.enums.macs_enum import MacsEnum
from app.common.logger import get_logger
from app.common.rpc.system_base_rpc import encrypt, delete_host_ubackup_agent
from app.common.util.cleaner import clear
from app.protection.object.models.projected_object import ProtectedObject
from app.resource.common.common_enum import HostMigrationIpType, HostMigrationOsType, ProxyHostTypeEnum
from app.resource.common.constants import HostMigrateConstants, HostMigrateJobSteps
from app.resource.db.db_res_db_api import query_host_info_by_uuid
from app.resource.kafka import topics
from app.resource.models.resource_models import EnvironmentTable, ResExtendInfoTable, TClusterTarget, ResourceTable
from app.resource.rpc.target_cluster_rpc import TargetClusterRpc
from app.resource.schemas.host_models import HostMigrationSchedule, HostMigrationUnexpectedExecuteReq
from app.resource.service.common import domain_resource_object_service
from app.resource.service.common.resource_service import query_target_cluster_by_id
from app.resource.service.host.host_res_service import delete_host, get_host_by_id, check_resource_has_sla
from app.resource.service.host.host_service import update_host_online, get_environment

log = get_logger(__name__)


def update_host_status_migrate(host_id):
    try:
        with database.session() as db:
            # 置主机状态为迁移中
            db.query(EnvironmentTable).filter(
                EnvironmentTable.uuid == host_id).update({"link_status": str(HostOnlineStatus.MIGRATE.value)})
    except Exception as ex:
        log.error(f"update host status is migrate error with id: { host_id}.")
        raise EmeiStorBizException(CommonErrorCodes.STATUS_ERROR) from ex
    finally:
        pass


def get_ip_type(ip):
    default_ip_type = None
    try:
        if isinstance(ip_address(ip), IPv4Address):
            return HostMigrationIpType.IPV4.value
        if isinstance(ip_address(ip), IPv6Address):
            return HostMigrationIpType.IPV6.value
        return default_ip_type
    except Exception:
        log.exception(f"get ip type fail")
        return "error"
    finally:
        pass


def get_os_type(os_type):
    # 返回大写的LINUX、UNIX、WINDOWS
    if os_type in ResourceConstant.LINUX_HOST_OS_TYPE_LIST:
        return HostMigrationOsType.LINUX.value
    if os_type in ResourceConstant.UNIX_HOST_OS_TYPE_LIST:
        return HostMigrationOsType.UNIX.value
    return os_type.upper()


def set_target_cluster_token(target_cluster_info):
    cluster_ip = target_cluster_info.cluster_ip.split(",")[0]
    token = ""
    try:
        token = TargetClusterRpc.get_target_cluster_token(ip=cluster_ip,
                                                          params={"userName": target_cluster_info.username,
                                                                  "password": target_cluster_info.password})
    except Exception as ex:
        log.exception(f"Get token: fail error:{ex}")
    finally:
        pass

    if not token:
        log.error(f"Get token: fail")
        # 修改为目标集群异常
        raise EmeiStorBizException(ResourceErrorCodes.CLUSTER_NODES_QUERY_FAILED,
                                   message="The network connection is abnormal or the target cluster is abnormal.")
    set_target_cluster_token_to_redis(cluster_ip, token)


def set_one_hour_host_migrate_end_cycle_to_redis(host_id, status='', expire=False):
    log.info(f"set one hour cycle host migrate id: {host_id} to redis status: {status}")
    context = Context('add_one_hour_host_migrate_' + str(host_id))
    context.set('host_id', host_id)
    context.set('job_status', status)
    # 设置过期时间
    if expire:
        context.expire(HostMigrateConstants.ONE_HOUR)


def is_host_migrate_one_hour_schedule_status(host_id, status='') -> bool:
    # 是否存在一小时任务状态
    context = Context('add_one_hour_host_migrate_' + str(host_id))
    if len(context.exist()) != 0:
        if status:
            return context.get('status')
        return True
    return False


def set_target_cluster_token_to_redis(target_cluster_ip, token):
    log.info(f"set target: {target_cluster_ip} cluster token redis")
    context = Context('add_target_cluster_tk_' + str(target_cluster_ip))
    context.set('token', token)
    context.set('ip', target_cluster_ip)
    # 过期时间,三天后自动消失。
    context.expire(HostMigrateConstants.THREE_DAY)
    clear(token)


def check_has_associated_protected_objects(agent_list: dict):
    with database.session() as session:
        condition = {
            ProtectedObject.sub_type == ResourceSubTypeEnum.VirtualMachine.value,
        }
        protection_object_vmware = session.query(ProtectedObject).filter(*condition).all()
        protection_name_list = []
        agent_name_list = []
        for protection_object in protection_object_vmware:
            vmware_agent_id = json.loads(str(protection_object.ext_parameters)).get("proxy_id", None)
            if vmware_agent_id in agent_list:
                protection_name_list.append(protection_object.name)
                agent_name_list.append(agent_list.get(vmware_agent_id))
                # VM 筛选后
                del agent_list[vmware_agent_id]

        for agent_id in agent_list:
            condition_u = {
                ResExtendInfoTable.key == "agents",
                ResExtendInfoTable.value.like(f'%{agent_id}%')
            }
            # 查询出扩展字段的中保护对象与主机关联表
            res_extend_info = session.query(ResExtendInfoTable).filter(*condition_u).all()
            resource_id_list = list(info.resource_id for info in res_extend_info)
            res = session.query(ResourceTable.name).filter(ResourceTable.uuid.in_(resource_id_list)).all()
            resource_list = list(name[0] for name in res)
            if resource_list:
                agent_name_list.append(agent_list.get(agent_id))

            protection_name_list += resource_list
        if agent_name_list:
            raise EmeiStorBizException(
                ResourceErrorCodes.MIGRATE_HOST_HAS_PROTECTED_OBJECTS,
                *[",".join(agent_name_list), ",".join(protection_name_list)])


def delete_host_choose(host, session):
    log.info(f"Start to delete_host_choose.")
    if host["sub_type"] in [ResourceSubTypeEnum.UBackupAgent.value, ResourceSubTypeEnum.S_BACKUP_AGENT.value]:
        try:
            # 通用外置代理走v2主机删除接口，与页面保持一致
            log.info(f"begin Start to delete_host_choose.")
            delete_host_ubackup_agent(host['host_id'])
            log.info(f"finish delete_host_choose.")
        except Exception as ex:
            # 正常情况不会走此分支，异常时便于定位
            log.exception(f"delete ubackup agent error: {ex}")
    else:
        # oracle/VM主机删除接口
        delete_host(host, session)
    log.info(f"delete host success :{host['host_id']}")


class HostMigrateObjectService:

    @staticmethod
    def check_target_cluster(target_cluster_info: TClusterTarget):
        # 校验目标集群是否在线
        if not target_cluster_info:
            # 修改为目标集群异常
            raise EmeiStorBizException(ResourceErrorCodes.CLUSTER_NODES_QUERY_FAILED,
                                       message="The network connection is abnormal or the target cluster is abnormal.")
        if target_cluster_info.status == HostMigrateConstants.TARGET_CLUSTER_STATUS_ABNORMAL:
            raise EmeiStorBizException(ResourceErrorCodes.CLUSTER_NODES_QUERY_FAILED,
                                       message="The network connection is abnormal or the target cluster is abnormal.")

        if target_cluster_info.role == HostMigrateConstants.REPLICATION_CLUSTER_TYPE:
            raise EmeiStorBizException(ResourceErrorCodes.MIGRATE_HOST_IS_REPLICATION_CLUSTER)

    @staticmethod
    def check_host_migrate(host_migrate):
        # 校验主机信息: 在线.是否存在sla
        host_info: EnvironmentTable = get_environment(host_migrate.host_id)
        if not host_info:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["hostId"])
        if host_info.link_status != str(HostOnlineStatus.ON_LINE.value):
            raise EmeiStorBizException(ResourceErrorCodes.RESOURCE_LINKSTATUS_OFFLINE)
        # 主机不存在保护,只针对资源
        with database.session() as session:
            check_resource_has_sla(session, host_migrate.host_id)
        return host_info.name

    @staticmethod
    def create_migrate_object(user_id: str, host_migrate_reqs):
        target_cluster_info = query_target_cluster_by_id(host_migrate_reqs.target_cluster_id)
        # 校验入参
        log.info(f"Start to check_target_cluster.")
        HostMigrateObjectService.check_target_cluster(target_cluster_info)
        agent_infos = {}
        for host_migrate_req in host_migrate_reqs.host_migrate_req:
            host_name = HostMigrateObjectService.check_host_migrate(host_migrate_req)
            # 校验主机Vm和通用外置代理是否被保护对象关联
            agent_infos[host_migrate_req.host_id] = host_name
            # 校验消息签名算法是否符合规范
            HostMigrateObjectService.check_macs(host_migrate_req)
        if agent_infos:
            log.info(f"Start to check_has_associated_protected_objects.")
            check_has_associated_protected_objects(agent_infos)

        job_list = []
        set_target_cluster_token(target_cluster_info)
        for host_migrate_req in host_migrate_reqs.host_migrate_req:
            host_info = get_environment(host_migrate_req.host_id)
            if not host_info:
                raise IllegalParamException(
                    CommonErrorCodes.ILLEGAL_PARAMS, ["hostId"])

            os_type = get_os_type(host_info.os_type)
            log.info(f"os_type = {os_type}")
            # windows 场景尚不支持
            if HostMigrationOsType.WINDOWS.value == os_type:
                raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR,
                                           message="windows is not support")

            # 检查用户自定义安装目录
            if host_migrate_reqs.install_path and host_migrate_reqs.should_check_install_path_permit_high:
                params = build_host_migration_param(host_info, host_migrate_req, str(uuid.uuid4()), target_cluster_info,
                                                    host_migrate_reqs.install_path)
                check_connect_param = HostMigrationSchedule(**params)
                check_connect_result = HostMigrateObjectService.post_target_cluster_agent_migrate_connection(
                    check_connect_param)
                if check_connect_result:
                    result = check_connect_result.get("result")
                    if result == "INSTALL_PATH_PERMIT_TOO_HIGH":
                        return check_connect_result
            # 创建job任务
            log.info(f"Start to create_host_migrate_job task.")
            job_id = HostMigrateObjectService.create_host_migrate_job(host_info, user_id, target_cluster_info)

            log.info(f"Start to create_host_migrate_immediate_schedule.")
            HostMigrateObjectService.create_host_migrate_immediate_schedule(host_info, host_migrate_req, job_id,
                                                                            target_cluster_info,
                                                                            host_migrate_reqs.install_path)
            # 异常情况下2天结束任务
            SchedulerClient.submit_interval_job(job_action=topics.MIGRATE_HOST_UNEXPECTED_END_SCHEDULE,
                                                job_name=host_migrate_req.host_id + "_unexpected",
                                                job_params=HostMigrationUnexpectedExecuteReq(job_id=job_id,
                                                                                             host_id=host_info.uuid),
                                                interval=HostMigrateConstants.ONE_HOUR)
            log.info(f"Start to job_list.append(job_id).")
            job_list.append(job_id)
        return job_list

    @staticmethod
    def check_macs(host_migrate_req):
        if host_migrate_req.ssh_macs is not None:
            if host_migrate_req.ssh_macs not in MacsEnum.__members__.values():
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["ssh macs is wrong."])

    @staticmethod
    def create_host_migrate_immediate_schedule(host_info, host_migrate_req, job_id, target_cluster_info, install_path):
        execute_req = build_host_migration_param(host_info, host_migrate_req, job_id, target_cluster_info, install_path)
        try:
            SchedulerClient.create_immediate_schedule(topic=topics.MIGRATE_HOST_IMMEDIATE_SCHEDULE, params=execute_req)
            # 将主机的状态更新为迁移中
            update_host_status_migrate(host_id=host_info.uuid)
        except Exception as ex:
            log.exception(f"create immediate schedule fail error: {ex}")
        finally:
            clear(execute_req.get('host_password'))
            clear(host_migrate_req.host_password)

    @staticmethod
    def create_host_migrate_job(host_info, user_id, target_cluster_info):
        request_id = str(uuid.uuid4())
        domain_id_list = domain_resource_object_service.get_domain_id_list(host_info.uuid)
        job_id = toolkit.create_job_center_task(request_id, {
            "requestId": request_id,
            "sourceId": host_info.uuid,
            "sourceName": host_info.name,
            "sourceType": host_info.type,
            "sourceSubType": HostMigrateConstants.PROTECT_AGENT,
            "sourceLocation": host_info.endpoint,
            "type": JobType.MIGRATE.value,
            "userId": user_id,
            "domainIdList": domain_id_list,
            "enableStop": False,
            # 目标集群位置
            "targetLocation": target_cluster_info.cluster_ip,
            # 目标集群名称
            "targetName": target_cluster_info.cluster_name
        })
        if not job_id:
            raise EmeiStorBizException(
                error=CommonErrorCodes.SYSTEM_ERROR, message="create host migrate job failed.")
        return job_id

    @staticmethod
    def post_target_cluster_agent_migrate(host_migrate_post_param: HostMigrationSchedule):
        # 发送请求到目标端 ，处理返回结果，接收到的是目标端任务id
        return TargetClusterRpc.post_target_cluster_migrate_task_rpc(host_migrate_post_param)

    @staticmethod
    def host_migrate_interval_schedule_task(host_migrate_schedule):
        write_redis_flag = False
        job_info = {}
        try:
            job_info = TargetClusterRpc.query_job_task_detail_rpc(host_migrate_schedule)
        except Exception as es:
            # 查询目标集群失败，不做处理。 目的：兼容目标集群重启等中断异常。如果第一次查就失败，任务结束。
            log.exception(f"Post target cluster agent migrate fail. error: {es}")
            HostMigrateObjectService.end_task_in_onr_hour(host_migrate_schedule)
            return
        finally:
            pass

        if job_info.get("totalCount") == 1:
            records = job_info.get("records")[0]
        else:
            records = {}
            log.error("target cluster job: error")
        # 根据查询到的当前目标集群任务的状态分别处理
        status = records.get("status")
        if status in [JobStatus.PENDING.value, JobStatus.READY.value]:
            write_redis_flag = HostMigrateObjectService.host_migrate_schedule_task_pending(host_migrate_schedule)

        elif status == JobStatus.RUNNING.value:
            write_redis_flag = HostMigrateObjectService.host_migrate_schedule_task_running(host_migrate_schedule)

        elif status == JobStatus.SUCCESS.value:
            HostMigrateObjectService.host_migrate_schedule_task_success(host_migrate_schedule)

        elif status == JobStatus.FAIL.value:
            write_redis_flag = HostMigrateObjectService.host_migrate_schedule_task_fail(host_migrate_schedule)

        else:
            log.error(f"Unexpected status value : {status}")

        if not write_redis_flag:
            # 不执行1小时判断:刚写入redis、失败后。
            HostMigrateObjectService.end_task_in_onr_hour(host_migrate_schedule)

    @staticmethod
    def end_task_in_onr_hour(host_migrate_schedule):
        if not is_host_migrate_one_hour_schedule_status(host_migrate_schedule.host_id):
            # 更新主机为在线 不存标识时强制结束，redis设置时设置了1小时过期时间
            update_host_online(host_migrate_schedule.host_id)
            HostMigrateObjectService.clear_migrate_schedule_and_context(host_migrate_schedule.host_id)
            HostMigrateObjectService.record_job_step(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                     HostMigrateJobSteps.HOST_MIGRATE_FAILED, JobLogLevel.FATAL)
            HostMigrateObjectService.update_job(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                JobStatus.FAIL)
        # 1小时后强制结束任务。

    @staticmethod
    def host_migrate_schedule_task_pending(host_migrate_schedule):
        # 准备中不记录任务周期时间
        if not is_host_migrate_one_hour_schedule_status(host_migrate_schedule.host_id, status=JobStatus.PENDING.value):
            # 如果redis不存在迁移标识,才写入迁移标识为任务等待中，如果存在了肯定是等待以后了
            set_one_hour_host_migrate_end_cycle_to_redis(host_migrate_schedule.host_id, JobStatus.PENDING.value)
            # 更新任务为排队中
            HostMigrateObjectService.record_job_step(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                     HostMigrateJobSteps.HOST_MIGRATE_PENDING)
            HostMigrateObjectService.update_job(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                JobStatus.PENDING)
            return True
        return False

    @staticmethod
    def host_migrate_schedule_task_fail(host_migrate_schedule):
        # 任务失败处理，更新主机为在线 ,删除调度，删除redis

        job_task_detail_log = TargetClusterRpc.query_job_task_detail_log_rpc(host_migrate_schedule)
        error_log = list(detail_log for detail_log in job_task_detail_log if
                         detail_log.get("level", "") == JobLogLevel.ERROR.value)

        if len(error_log) == 0:
            # 没有查到错误信息
            HostMigrateObjectService.record_job_step(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                     HostMigrateJobSteps.HOST_MIGRATE_FAILED, JobLogLevel.FATAL)

        elif len(error_log) == 1:
            # 错误信息只有一步
            # 将推送安装错误信息日志写入到本地集群迁移任务中
            HostMigrateObjectService.record_job_step(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                     HostMigrateJobSteps.HOST_MIGRATE_FAILED, JobLogLevel.FATAL,
                                                     log_detail=error_log[0].get('logDetail'),
                                                     log_detail_param=error_log[0].get('logDetailParam'))

        else:
            log.error(f"There should be no more than one error log: {error_log}")
        # 失败后更新为在线
        update_host_online(host_migrate_schedule.host_id)
        # 清理redis和调度
        HostMigrateObjectService.clear_migrate_schedule_and_context(host_migrate_schedule.host_id)
        # 更新任务为失败
        HostMigrateObjectService.update_job(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                            JobStatus.FAIL)
        return True

    @staticmethod
    def host_migrate_schedule_task_success(host_migrate_schedule):
        HostMigrateObjectService.record_job_step(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                 HostMigrateJobSteps.HOST_MIGRATE_TASK_FINISH)

        # 更新任务相关状态,迁移完成默认主机是离线（如果为在线需要对应主机应用去操作），删除本地相关资源,删除调度，删除redis
        host = get_host_by_id(host_migrate_schedule.host_id)

        try:
            with database.session() as db:
                delete_host_choose(host, db)
                log.info(f"host: {host_migrate_schedule.host_id} migrate success")
                HostMigrateObjectService.record_job_step(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                         HostMigrateJobSteps.HOST_MIGRATE_CLEAR_RESOURCE)
                HostMigrateObjectService.update_job(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                    JobStatus.RUNNING, 95)

        except EmeiStorBizException as es:
            # 删除主机任务失败
            log_detail = es.error_code if isinstance(es, EmeiStorBizException) else None
            log_detail_param = es.parameter_list if isinstance(es, EmeiStorBizException) else None
            log.exception(f"clear host resource: fail")
            # 记录删除主机失败。
            HostMigrateObjectService.record_job_step(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                     HostMigrateJobSteps.HOST_MIGRATE_CLEAR_FAILED_RESOURCE,
                                                     JobLogLevel.WARNING, log_detail, log_detail_param)
        finally:
            # 主机迁移任务成功,
            HostMigrateObjectService.record_job_step(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                     HostMigrateJobSteps.HOST_MIGRATE_TASK_SUCCESS)
            HostMigrateObjectService.update_job(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                JobStatus.SUCCESS)
            HostMigrateObjectService.clear_migrate_schedule_and_context(host_migrate_schedule.host_id)

    @staticmethod
    def host_migrate_schedule_task_running(host_migrate_schedule):
        if not is_host_migrate_one_hour_schedule_status(host_migrate_schedule.host_id):
            # 不存在创建，任务开始时记录时间周期为1小时，存入redis，设置1小时超时
            set_one_hour_host_migrate_end_cycle_to_redis(host_migrate_schedule.host_id, expire=True)
            HostMigrateObjectService.record_job_step(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                     HostMigrateJobSteps.HOST_MIGRATE_POST_TASK_FINISH,
                                                     log_info_param=[host_migrate_schedule.ip_address])
            HostMigrateObjectService.update_job(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                JobStatus.RUNNING, 20)
            return True

        if is_host_migrate_one_hour_schedule_status(host_migrate_schedule.host_id,
                                                    status=JobStatus.RUNNING.value) == JobStatus.PENDING.value:
            # 上一个状态为PENDING，覆盖之前的值,任务开始时记录时间周期为1小时，存入redis，设置1小时超时。
            set_one_hour_host_migrate_end_cycle_to_redis(host_migrate_schedule.host_id, expire=True)
            HostMigrateObjectService.record_job_step(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                     HostMigrateJobSteps.HOST_MIGRATE_POST_TASK_FINISH,
                                                     log_info_param=[host_migrate_schedule.ip_address])
            HostMigrateObjectService.update_job(host_migrate_schedule.job_id, host_migrate_schedule.job_id,
                                                JobStatus.RUNNING, 20)
            return True
        return False

    @staticmethod
    def host_migrate_immediate_tasks(request_id, host_migrate_schedule):
        # 更新任务为准备中
        HostMigrateObjectService.record_job_step(host_migrate_schedule.job_id, request_id,
                                                 HostMigrateJobSteps.HOST_MIGRATE_READY_CREATE_CREATE)
        HostMigrateObjectService.update_job(host_migrate_schedule.job_id, request_id, JobStatus.READY)
        # 发送post请求到目标目前，下发迁移任务。
        target_cluster_job_id = ''
        try:
            target_cluster_job_id = HostMigrateObjectService.post_target_cluster_agent_migrate(host_migrate_schedule)
        except EmeiStorBizException as es:
            log.error("host_migrate_immediate_tasks error.")
            # 发送任务失败根据返回的失败信息更新任务。
            log_detail = es.error_code if isinstance(es, EmeiStorBizException) else None
            log_detail_param = es.parameter_list if isinstance(es, EmeiStorBizException) else None
            # 更新迁移任务失败
            HostMigrateObjectService.record_job_step(job_id=host_migrate_schedule.job_id, request_id=request_id,
                                                     job_step_label=HostMigrateJobSteps.HOST_MIGRATE_FAILED,
                                                     log_detail=log_detail, log_detail_param=log_detail_param,
                                                     log_level=JobLogLevel.FATAL)
            HostMigrateObjectService.update_job(host_migrate_schedule.job_id, request_id, JobStatus.FAIL)
            # 更新主机为在线
            update_host_online(host_migrate_schedule.host_id)
            HostMigrateObjectService.clear_migrate_schedule_and_context(host_migrate_schedule.host_id)
            return
        finally:
            pass

        if not target_cluster_job_id:
            log.error("get target cluster job id fail.")
            raise EmeiStorBizException(
                error=CommonErrorCodes.SYSTEM_ERROR, message="get target cluster job id fail.")

        # 发送轮询调度任务
        HostMigrateObjectService.create_migrate_schedule(target_cluster_job_id, host_migrate_schedule)

    @staticmethod
    def create_migrate_schedule(target_cluster_job_id, execute_req: HostMigrationSchedule):
        # 创建定时轮询任务，60s
        execute_req.target_cluster_job_id = target_cluster_job_id
        try:
            # 如果存在该任务（job_name）：会覆盖原有任务。
            SchedulerClient.submit_interval_job(job_action=topics.MIGRATE_HOST_INTERVAL_SCHEDULE,
                                                job_name=execute_req.host_id+"_interval", job_params=execute_req,
                                                interval=HostMigrateConstants.INTERVAL)
        except Exception:
            log.exception(f"create submit interval job failed, job_name: {execute_req.host_id+'_interval'}.")
            client = SchedulerClient()
            client.batch_delete_schedules([execute_req.host_id])
        finally:
            pass

    @staticmethod
    def record_job_step(job_id: str, request_id: str, job_step_label: str,
                        log_level: JobLogLevel = JobLogLevel.INFO,
                        log_info_param=None, log_detail=None, log_detail_param=None):
        """
        job_id: 任务id request_id: 请求id job_step_label: 任务日志标签 log_level:
        日志级别：正常：info 警告：warning 错误：warning 严重错误：FATAL  log_info_param:日志标签下信息
        log_detail:详细日志 log_detail_param:详细日志更多
        """
        if not log_info_param:
            log_info_param = []
        log_step_req = toolkit.build_update_job_log_request(job_id, job_step_label, log_level, log_info_param,
                                                            log_detail, log_detail_param)
        toolkit.modify_task_log(request_id, job_id, log_step_req)

    @staticmethod
    def update_job(job_id: str, request_id: str, status: JobStatus, progress=None):
        update_req = {
            "status": status.value
        }
        if status is JobStatus.RUNNING:
            update_req.update(progress=progress)
        elif status is JobStatus.SUCCESS or status is JobStatus.PARTIAL_SUCCESS:
            update_req.update(progress=100)
        toolkit.modify_task_log(request_id, job_id, update_req)

    @staticmethod
    def clear_migrate_schedule_and_context(host_id):
        if is_host_migrate_one_hour_schedule_status(host_id):
            # 清理周期调度redis信息
            Context('add_one_hour_host_migrate_' + str(host_id)).delete_all()
        # 清理调度器

        client = SchedulerClient()
        client.batch_delete_schedules([host_id+"_interval"])
        client.batch_delete_schedules([host_id + "_unexpected"])


    @staticmethod
    def post_target_cluster_agent_migrate_connection(host_migrate_post_param: HostMigrationSchedule):
        # 发送请求到目标端 ，处理返回结果
        return TargetClusterRpc.post_target_cluster_migrate_task_check_connect_rpc(host_migrate_post_param)


# 迁移主机公共函数
def get_migrate_host_ip_by_params(params):
    host_migrate_list = params.get("host_migrate_req", {})

    if not host_migrate_list:
        return "--"
    ip_list = []
    for host_migrate in host_migrate_list.host_migrate_req:
        host_info = query_host_info_by_uuid(host_migrate.host_id)
        if host_info:
            ip_list.append(host_info.endpoint)
    if ip_list:
        return ",".join(ip_list)
    return "--"


def build_host_migration_param(host_info, host_migrate_req, job_id, target_cluster_info, install_path):
    return {
        "host_id": host_info.uuid,
        "ip_address": host_info.endpoint,
        "proxy_host_type": ProxyHostTypeEnum.get_type_by_proxy(host_info.sub_type),
        "os_type": get_os_type(host_info.os_type),
        "ip_type": get_ip_type(host_info.endpoint),
        "host_user_name": host_migrate_req.host_user_name,
        "host_password": encrypt(host_migrate_req.host_password),
        "ssh_macs": host_migrate_req.ssh_macs,
        "business_ip_flag": host_migrate_req.business_ip_flag,
        "port": host_migrate_req.port,
        "target_cluster_ip": target_cluster_info.cluster_ip.split(",")[0],
        "target_cluster_port": target_cluster_info.cluster_port,
        "job_id": job_id,
        "target_cluster_job_id": '',
        "install_path": install_path
    }
