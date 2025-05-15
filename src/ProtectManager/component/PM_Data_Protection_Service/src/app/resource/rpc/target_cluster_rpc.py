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
from http import HTTPStatus

import requests
from fastapi import HTTPException

from app.common.context.context import Context
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack
from app.common.logger import get_logger
from app.common.rpc.system_base_rpc import decrypt, encrypt
from app.common.util.cleaner import clear
from app.resource.common.common_enum import MigrateType
from app.resource.common.constants import HostMigrateConstants
from app.resource.schemas.host_models import HostMigrationSchedule

log = get_logger(__name__)

RESPONSE_ERROR_KEY = [400, 500]


def is_response_status_ok(response: requests.Response):
    if response:
        return response.status_code < 400
    return False


def build_host_migrate_param(host_migrate_schedule: HostMigrationSchedule):
    if host_migrate_schedule.host_password:
        host_migrate_schedule.host_password = decrypt(host_migrate_schedule.host_password)
    return {
        "ips": host_migrate_schedule.ip_address,
        "type": host_migrate_schedule.proxy_host_type,
        "osType": host_migrate_schedule.os_type,
        "ipType": host_migrate_schedule.ip_type,
        "username": host_migrate_schedule.host_user_name,
        "password": host_migrate_schedule.host_password,
        "macs": host_migrate_schedule.ssh_macs,
        "migration": MigrateType.HOST_MIGRATE.value,
        "ipInfos": [
            {
                "ip": host_migrate_schedule.ip_address,
                "businessIpFlag": host_migrate_schedule.business_ip_flag,
                "port": host_migrate_schedule.port
            }
        ],
        "installPath": host_migrate_schedule.install_path
    }


def get_target_cluster_token_by_ip_from_redis(target_cluster_ip):
    default_token = None
    context = Context('add_target_cluster_tk_' + str(target_cluster_ip))
    if len(context.exist()) != 0:
        token = context.get('token')
        return token
    return default_token


def post_migration_task(host_migration_req: HostMigrationSchedule, url, token):
    if ":" in host_migration_req.target_cluster_ip:
        host_migration_req.target_cluster_ip = f'[{host_migration_req.target_cluster_ip}]'
    if not token:
        log.error(f"[Migrate Task] get token failed from redis")
        raise EmeiStorBizException(ResourceErrorCodes.CLUSTER_NODES_QUERY_FAILED, message="get token failed")
    response = requests.request("POST", url, headers={
        "x-auth-token": decrypt(token),
        **HostMigrateConstants.COMMON_HEADERS
    }, data=json.dumps(build_host_migrate_param(host_migration_req)), verify=False)
    response.encoding = "utf-8"
    return response


class TargetClusterRpc:

    @staticmethod
    def get_target_cluster_token(ip, params: dict):
        if ":" in ip:
            ip = f'[{ip}]'
        url = f'https://{ip}:{HostMigrateConstants.TARGET_ClUSTER_PORT}/v1/auth/token'
        if params.get("password"):
            # 解密密码
            params["password"] = decrypt(params.get("password"))
        try:
            response = requests.request("POST", url, headers=HostMigrateConstants.COMMON_HEADERS,
                                        data=json.dumps(params), verify=False)
            response.encoding = "utf-8"
        except HTTPException as ex:
            log.exception(f"[Get target cluster]: token failed ip: {ip}.")
            raise ex
        finally:
            clear(params.get("password"))

        if is_response_status_ok(response):
            # 返回加密的token
            return encrypt(response.json().get("token", ""))
        else:
            raise HTTPException(response.status_code)

    @staticmethod
    @exter_attack
    def post_target_cluster_migrate_task_rpc(host_migration_req: HostMigrationSchedule):
        url = f'https://{host_migration_req.target_cluster_ip}:{HostMigrateConstants.TARGET_ClUSTER_PORT}' \
              f'/v1/host-agent/register'
        token = get_target_cluster_token_by_ip_from_redis(host_migration_req.target_cluster_ip)
        try:
            response = post_migration_task(host_migration_req, url, token)
        except Exception as ex:
            log.exception(f"[Post target cluster] migrate failed ip: {host_migration_req.ip_address} error: {ex}")
            # 修改为目标集群异常
            raise EmeiStorBizException(ResourceErrorCodes.CLUSTER_NODES_QUERY_FAILED,
                                       message="post target cluster migrate failed") from ex
        finally:
            clear(host_migration_req.host_password)
            clear(token)

        if is_response_status_ok(response):
            # 解析返回任务的详情转化为自己任务
            target_cluster_job_id = response.json()
            if isinstance(target_cluster_job_id, list):
                # 返回值为列表时，默认只去第一个。
                target_cluster_job_id = target_cluster_job_id[0]
            return target_cluster_job_id
        else:
            log.error(f'errorCode: {response.json().get("errorCode", "")} '
                      f'errorMessage: {response.json().get("errorMessage", "")}')
            if response.status_code in RESPONSE_ERROR_KEY:
                # 解析主机推送安装发生错误。
                TargetClusterRpc.raise_pm_post_error(response)
            # 其他原因导致连接目标集群失败
            raise EmeiStorBizException(error=ResourceErrorCodes.CLUSTER_NODES_QUERY_FAILED,
                                       message="get target cluster job id fail.")

    @staticmethod
    @exter_attack
    def raise_pm_post_error(response):
        raise EmeiStorBizException.build_from_error(response.json())

    @staticmethod
    @exter_attack
    def query_job_task_detail_rpc(host_migrate_schedule: HostMigrationSchedule) -> dict:
        """
        return:任务
        "totalCount": 1,
        "records": [
                {
                "userId": "88a94c476f12a21e016f12a246e50009",
                "jobId": "ed7a06d4-01ba-4484-a70d-634aac18e91c",
                "type": "resource_scan",
                "progress": 10,
                "startTime": 1646272901800,
                "endTime": 1646272904375,
                "lastUpdateTime": 1646272904443,
                "status": "FAIL" job状态：主机注册只有四个状态PENDING-排队中、RUNNING-运行中、SUCCESS-成功、FAIL-失败
                }
        """
        if ":" in host_migrate_schedule.target_cluster_ip:
            # ipv6
            host_migrate_schedule.target_cluster_ip = f'[{host_migrate_schedule.target_cluster_ip}]'
        url = f'https://{host_migrate_schedule.target_cluster_ip}:{HostMigrateConstants.TARGET_ClUSTER_PORT}' \
              f'/v1/jobs?jobId={host_migrate_schedule.target_cluster_job_id}'
        token = get_target_cluster_token_by_ip_from_redis(host_migrate_schedule.target_cluster_ip)
        if not token:
            log.error(f"[Cluster Task]: get token failed form redis")

        try:
            response = requests.request("GET", url, headers={
                "x-auth-token": decrypt(token),
                **HostMigrateConstants.COMMON_HEADERS
            }, verify=False)
            response.encoding = "utf-8"
        except Exception as ex:
            log.exception(
                f"Get target cluster token failed, ip: {host_migrate_schedule.target_cluster_ip}. error: {ex}")
            raise EmeiStorBizException(ResourceErrorCodes.CLUSTER_NODES_QUERY_FAILED) from ex
        finally:
            clear(token)

        if is_response_status_ok(response):
            # 解析返回任务的详情转化为自己任务
            return response.json()
        else:
            log.error(f'error_code: {response.json().get("error_code", "")} '
                      f'errorMessage: {response.json().get("errorMessage", "")}')
            raise HTTPException(response.status_code)

    @staticmethod
    @exter_attack
    def query_job_task_detail_log_rpc(host_migrate_schedule: HostMigrationSchedule):
        """
        {
        "totalCount": 2,
        "records": [
                            {
                "jobId": "任务ID",
                "startTime": 开始时间：1646272904245,
                "endTime": null,
                "logInfo": "错误标签：job_log_agent_register_fail_label",
                "level": "日志级别：error",
                "logInfoParam": [],
                "logDetail": "错误码",
                "logDetailParam": [],
                "logDetailInfo": null
            },
        """
        # ipv6
        if ":" in host_migrate_schedule.target_cluster_ip:
            host_migrate_schedule.target_cluster_ip = f'[{host_migrate_schedule.target_cluster_ip}]'
        url = f'https://{host_migrate_schedule.target_cluster_ip}:{HostMigrateConstants.TARGET_ClUSTER_PORT}' \
              f'/v1/jobs/{host_migrate_schedule.target_cluster_job_id}/logs'
        token = get_target_cluster_token_by_ip_from_redis(host_migrate_schedule.target_cluster_ip)
        if not token:
            log.error(f"[Cluster Task Detail] get token failed form redis")

        try:
            response = requests.request("GET", url, headers={
                "x-auth-token": decrypt(token),
                **HostMigrateConstants.COMMON_HEADERS
            }, verify=False)
            response.encoding = "utf-8"
        except HTTPException as ex:
            log.exception(f"Get target cluster token failed, ip: {host_migrate_schedule.target_cluster_ip}.")
            raise ex
        finally:
            clear(token)

        if is_response_status_ok(response):
            # 解析返回任务的详情转化为自己任务
            return response.json().get("records", [])
        else:
            log.error(f'error_code: {response.json().get("error_code", "")} '
                      f'errorMessage: {response.json().get("errorMessage", "")}')
            raise HTTPException(response.status_code)

    @staticmethod
    @exter_attack
    def post_target_cluster_migrate_task_check_connect_rpc(host_migration_req: HostMigrationSchedule):
        url = f'https://{host_migration_req.target_cluster_ip}:{HostMigrateConstants.TARGET_ClUSTER_PORT}' \
              f'/v1/host-agent/register/connection'
        token = get_target_cluster_token_by_ip_from_redis(host_migration_req.target_cluster_ip)
        try:
            response = post_migration_task(host_migration_req, url, token)
        except Exception as ex:
            log.exception(f"[Post target cluster] migrate failed ip: {host_migration_req.ip_address} error: {ex}")
            # 修改为目标集群异常
            raise EmeiStorBizException(ResourceErrorCodes.CLUSTER_NODES_QUERY_FAILED,
                                       message="post target cluster migrate failed") from ex
        finally:
            clear(token)
        if not is_response_status_ok(response):
            log.error(f'errorCode: {response.json().get("errorCode", "")} '
                      f'errorMessage: {response.json().get("errorMessage", "")}')
            if response.status_code in RESPONSE_ERROR_KEY:
                # 解析主机推送安装发生错误。
                TargetClusterRpc.raise_pm_post_error(response)
            # 其他原因导致连接目标集群失败
            raise EmeiStorBizException(error=ResourceErrorCodes.CLUSTER_NODES_QUERY_FAILED,
                                       message="get target cluster job id fail.")
        if response.status_code == HTTPStatus.OK:
            if response.content:
                return response.json()
        return json.loads('{}')


