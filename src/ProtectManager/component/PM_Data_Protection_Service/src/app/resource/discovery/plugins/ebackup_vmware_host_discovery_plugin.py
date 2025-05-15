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
import time

from app.backup.client.rbac_client import RBACClient
from app.common.enums.rbac_enum import ResourceSetTypeEnum, ResourceSetScopeModuleEnum
from app.common.events.topics import RESOURCE_DELETED_TOPIC
from app.common.lock.lock import Lock
from app.common.lock.lock_manager import lock_manager
from app.common.logger import get_logger

from app.common.events.producer import produce
from app.base.db_base import database
import app.resource.service.host.host_service
from app.resource.client.system_base import get_user_info_by_user_id
from app.resource.service.common import resource_service
from app.resource.models.resource_models import ResourceTable, LinkStatusTable, EnvironmentTable
from app.resource.discovery.discovery_plugin import DiscoveryPlugin
from app.common.event_messages.Discovery.discovery_rest import HostOnlineStatus
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.event_messages.event import CommonEvent
from app.common.clients.alarm.alarm_schemas import ClearAlarmReq, SendAlarmReq
from app.common.clients.alarm.alarm_client import AlarmClient
from app.common.enums.alarm_enum import AlarmSourceType
from app.common.redis_session import redis_session
from app.resource.rpc import hw_agent_rpc
from app.base.resource_consts import ResourceConstant
from app.resource.schemas.env_schemas import ScanEnvSchema, UpdateEnvSchema
from app.resource.service.common.user_domain_service import get_domain_id_by_user_id
from app.resource.service.host.host_service import automatic_authorization_by_agent_userid, upsert_extend_info
from app.resource.common.constants import ResourceAlarmConst
from app.resource.service.host.host_service import get_domain_id_list
from app.common.schemas.resource_set_relation_schemas import ResourceSetRelationInfo

log = get_logger(__name__)
VMWARE_KEEP_CHECK_ALIVE_INTERVAL = 180
CHECK_VMWARE_AGENT_LINK_STATUS_RETRY_TIMES = 3

SERVICES = [ResourceSubTypeEnum.VMBackupAgent]


class VMwareBackupAgentDiscoveryPlugin(DiscoveryPlugin):
    service_type = ResourceSubTypeEnum.VMBackupAgent

    @staticmethod
    def generate_alarm_id_key(env_id: str):
        return ResourceAlarmConst.LINK_STATUS_ALARM_ID_KEY.format(resource_id=env_id)

    @staticmethod
    def clear_alarm(env_id: str):
        alarm_id = redis_session.get(
            ResourceAlarmConst.LINK_STATUS_ALARM_ID_KEY.format(resource_id=env_id))
        if alarm_id:
            AlarmClient.clear_alarm(ClearAlarmReq(entityIdSet=[alarm_id]))
            redis_session.delete(
                ResourceAlarmConst.LINK_STATUS_ALARM_ID_KEY.format(resource_id=env_id))

    @staticmethod
    def check_need_send_alarm_id(env_ip: str):
        # 考虑闪断情况，循环三次每次请求5s接口如果出现问题
        num = 0
        while num < CHECK_VMWARE_AGENT_LINK_STATUS_RETRY_TIMES:
            num += 1
            try:
                # 循环3次，里面请求2次，每次5秒
                hw_agent_rpc.query_vm_agent_info(env_ip)
                return False
            except Exception:
                time.sleep(5)
            finally:
                pass
        # 三次之后保证确实失联了
        return True

    @staticmethod
    def build_alarm_req(env_ip: str, user_id: str):
        timestamp = int(time.time())
        return SendAlarmReq(
            alarmId=ResourceConstant.ALARM_VMWARE_HOST_AGENT_LINK_STATUS_OFFLINE,
            params=env_ip,
            userId=user_id,
            alarmSource="localhost",
            createTime=timestamp,
            sequence=timestamp,
            sourceType=AlarmSourceType.RESOURCE
        )

    @staticmethod
    def delete_agent_check(host_info):
        # 校验vmware agent状态是否正确 离线才能删除否则不能删除
        if str(host_info.link_status) == str(HostOnlineStatus.ON_LINE.value):
            message = f"Cannot delete because host is online."
            raise EmeiStorBizException(
                CommonErrorCodes.STATUS_ERROR, message=message)

    @staticmethod
    def update_agent_link_status(host, agent_status):
        try:
            with database.session() as db:
                # 置主机状态为离线
                host = db.query(EnvironmentTable).filter(
                    EnvironmentTable.uuid == host.get("uuid")).update({EnvironmentTable.link_status: agent_status})
        except Exception as ex:
            log.error(
                f"[refresh host status error] uuid: {host.get('uuid')}.")
            raise EmeiStorBizException(CommonErrorCodes.STATUS_ERROR) from ex
        finally:
            pass

    @classmethod
    def do_modify_env(cls, params: UpdateEnvSchema):
        host_info = params.extend_context.get('host')
        domain_id = ""
        if host_info['user_id']:
            domain_id = get_domain_id_by_user_id(host_info['user_id'])
        env = EnvironmentTable(**host_info)
        with database.session() as session:
            session.merge(env)
            resource_set_relation_info = ResourceSetRelationInfo(resource_object_id=env.uuid,
                                                                 resource_set_type=ResourceSetTypeEnum.AGENT.value,
                                                                 scope_module=ResourceSetScopeModuleEnum.AGENT.value,
                                                                 domain_id_list=get_domain_id_list(domain_id))
            RBACClient.add_resource_set_relation(resource_set_relation_info)
        # 更新扩展表中的src_deduption
        extend_info = params.extend_context.get('extend_info')
        upsert_extend_info(env.uuid, extend_info)
        return params

    def do_scan_env(self, params: ScanEnvSchema, is_rescan=False, is_session_connect=False):
        env = params.extend_context.get('host')
        extend_info = params.extend_context.get('extend_info')
        if is_rescan:
            self.do_fetch_resource(params)
        else:
            app.resource.service.host.host_service.upsert_environment(env)
            app.resource.service.host.host_service.upsert_extend_info(params.uuid, extend_info)
        return env

    def do_delete_env(self, params: str):
        with database.session() as db:
            host_info = resource_service.query_resource_by_id(params)
            self.delete_agent_check(host_info)
            # 删除与之关联的link_status表里的连通性数据
            db.query(LinkStatusTable).filter(LinkStatusTable.source_addr ==
                                             host_info.endpoint).delete(synchronize_session=False)
            db.query(ResourceTable).filter(ResourceTable.uuid ==
                                           params).delete(synchronize_session=False)
            RBACClient.delete_resource_set_relation([params],
                                                    ResourceSetTypeEnum.AGENT.value)

        # 发送删除消息
        message = CommonEvent(RESOURCE_DELETED_TOPIC, resource_id=params)
        produce(message)

    def get_alarm_id(self, env_id: str):
        return redis_session.get(self.generate_alarm_id_key(env_id=env_id))

    def is_exist_alarm(self, env_id: str):
        return redis_session.exists(self.generate_alarm_id_key(env_id=env_id)) > 0

    def send_alarm(self, env_id: str, env_ip, user_id):
        # 兼容支持幂等
        alarm_id = redis_session.get(
            ResourceAlarmConst.LINK_STATUS_ALARM_ID_KEY.format(resource_id=env_id))
        if alarm_id:
            log.info(
                f"[VMware Host]: Agent: {env_id} link status alarm existed,don't need send again ")
            return
        # 增加分布式锁 保证多控不会同时触发告警
        lock: Lock = lock_manager.get_lock(key=ResourceAlarmConst.LINK_STATUS_ALARM_LOCK_KEY.format(resource_id=env_id))
        if lock.lock(timeout=VMWARE_KEEP_CHECK_ALIVE_INTERVAL - 2, blocking_timeout=1):
            try:
                alarm_id = AlarmClient.send_alarm(
                    self.build_alarm_req(env_ip=env_ip, user_id=user_id))
                if alarm_id:
                    redis_session.setnx(ResourceAlarmConst.LINK_STATUS_ALARM_ID_KEY.format(
                        resource_id=env_id), alarm_id)
            except Exception:
                log.exception(
                    f"[VMware Host]: Agent: {env_id} link status alarm send error")
            finally:
                lock.unlock()

    def host_online_clear_alarm(self, env):
        if self.is_exist_alarm(env_id=env['uuid']):
            # 清除告警标记
            self.clear_alarm(env_id=env['uuid'])

    def host_offline_send_alarm(self, env):
        if not self.is_exist_alarm(env_id=env['uuid']):
            # 不存在告警标记 但是需要观察闪断现象
            if self.check_need_send_alarm_id(env_ip=env['endpoint']):
                self.send_alarm(
                    env_id=env['uuid'], env_ip=env['endpoint'], user_id=env['user_id'])

    def do_fetch_resource(self, params: ScanEnvSchema):
        pass


def create():
    return VMwareBackupAgentDiscoveryPlugin()
