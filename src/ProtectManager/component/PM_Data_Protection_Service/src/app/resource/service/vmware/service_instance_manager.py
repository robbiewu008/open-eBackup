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
import threading
import time

from redis.lock import Lock

from app.base.resource_consts import ResourceConstant
from app.resource.schemas.env_schemas import ScanEnvSchema
from app.resource.service.common import resource_service
from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService
from app.resource.common.constants import ResourceAlarmConst
from app.common.clients.alarm.alarm_client import AlarmClient
from app.common.clients.alarm.alarm_schemas import ClearAlarmReq, SendAlarmReq
from app.common.clients.system_base_client import SystemBaseClient
from app.common.enums.alarm_enum import AlarmSourceType
from app.common.enums.resource_enum import LinkStatusEnum
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.logger import get_logger
from app.common.redis_session import redis_session
from app.common.util.cleaner import clear

logger = get_logger(__name__)

LINK_STATUS_ALARM_SEND_CHECK_TIME = 120
KEEP_CHECK_ALIVE_INTERVAL = 30


def reconnection_vcenter():
    try:
        vcenter_info = resource_service.query_resource({"type": "vSphere"})
        if not vcenter_info:
            return
        for vcenter in vcenter_info:
            service_instance_manager.get_service_instance(vcenter.get("uuid"))
    except Exception:
        logger.exception(f"Reconnect vcenter/esx error in rooter.")
    finally:
        pass


class EnvLinkStatusChecker:

    @staticmethod
    def generate_alarm_flag_key(env_id: str):
        return ResourceAlarmConst.LINK_STATUS_ALARM_FLAG_KEY.format(resource_id=env_id)

    @staticmethod
    def set_alarm_flag(env_id: str):
        redis_session.setnx(EnvLinkStatusChecker.generate_alarm_flag_key(env_id=env_id), int(time.time()))

    @staticmethod
    def get_alarm_flag(env_id: str):
        return redis_session.get(EnvLinkStatusChecker.generate_alarm_flag_key(env_id=env_id))

    @staticmethod
    def delete_alarm_flag(env_id: str):
        alarm_id = redis_session.get(ResourceAlarmConst.LINK_STATUS_ALARM_ID_KEY.format(resource_id=env_id))
        if alarm_id:
            redis_session.delete(ResourceAlarmConst.LINK_STATUS_ALARM_ID_KEY.format(resource_id=env_id))
        return redis_session.delete(EnvLinkStatusChecker.generate_alarm_flag_key(env_id=env_id))

    @staticmethod
    def is_exist_alarm_flag(env_id: str) -> bool:
        return redis_session.exists(EnvLinkStatusChecker.generate_alarm_flag_key(env_id=env_id)) > 0

    @staticmethod
    def check_need_send_alarm(env_id: str) -> bool:
        alarm_time = EnvLinkStatusChecker.get_alarm_flag(env_id=env_id)
        if alarm_time and int(time.time()) - int(alarm_time) < LINK_STATUS_ALARM_SEND_CHECK_TIME:
            return False
        else:
            return True

    @staticmethod
    def build_alarm_req(env_ip: str, env_name:str, user_id: str):
        timestamp = int(time.time())
        return SendAlarmReq(
            alarmId=ResourceConstant.ALARM_ENVIRONMENT_LINK_STATUS_OFFLINE,
            params="{env_name},{env_ip}".format(env_name=env_name, env_ip=env_ip),
            userId=user_id,
            alarmSource="localhost",
            createTime=timestamp,
            sequence=timestamp,
            sourceType=AlarmSourceType.RESOURCE
        )

    @staticmethod
    def send_alarm(env_id: str, env_ip: str, env_name:str, user_id: str):
        # 兼容支持幂等
        alarm_id = redis_session.get(ResourceAlarmConst.LINK_STATUS_ALARM_ID_KEY.format(resource_id=env_id))
        if alarm_id:
            logger.info(f"resource [{env_id}] link status alarm existed,don't need send again ")
            return
        # 增加分布式锁
        lock: Lock = redis_session.lock(
            name=ResourceAlarmConst.LINK_STATUS_ALARM_LOCK_KEY.format(resource_id=env_id),
            timeout=KEEP_CHECK_ALIVE_INTERVAL - 2,
            blocking_timeout=1,
            thread_local=False)
        if lock.acquire():
            try:
                alarm_id = AlarmClient.send_alarm(EnvLinkStatusChecker.build_alarm_req(env_ip=env_ip, env_name=env_name,
                                                                                       user_id=user_id))
                if alarm_id:
                    redis_session.setnx(ResourceAlarmConst.LINK_STATUS_ALARM_ID_KEY
                                        .format(resource_id=env_id), alarm_id)
            except Exception:
                logger.exception("resource [{env_id}] link status alarm send error")
            finally:
                lock.release()

    @staticmethod
    def clear_alarm(env_id: str):
        alarm_id = redis_session.get(ResourceAlarmConst.LINK_STATUS_ALARM_ID_KEY.format(resource_id=env_id))
        if alarm_id:
            AlarmClient.clear_alarm(ClearAlarmReq(entityIdSet=[alarm_id]))

    @staticmethod
    def handle_link_status_online(env_id, env_info):
        # 当前状态为在线，数据库状态为在线，未发生变化，不做处理
        if env_info.link_status == LinkStatusEnum.Online.value:
            return
        # 当前状态为在线，数据库状态为离线，状态发生变化
        if EnvLinkStatusChecker.is_exist_alarm_flag(env_id=env_id):
            EnvLinkStatusChecker.clear_alarm(env_id=env_id)
            # 清除告警标记
            EnvLinkStatusChecker.delete_alarm_flag(env_id=env_id)
        # 修改状态为连通
        resource_service.update_link_status(env_id_list=[env_id], link_status=LinkStatusEnum.Online)

    @staticmethod
    def handle_link_status_offline(env_id, env_info):
        user_id = env_info.user_id if env_info.user_id else None
        if env_info.link_status == LinkStatusEnum.Offline.value:
            if EnvLinkStatusChecker.is_exist_alarm_flag(env_id=env_id):
                # 存在告警标记
                if EnvLinkStatusChecker.check_need_send_alarm(env_id=env_id):
                    EnvLinkStatusChecker.send_alarm(env_id=env_id, env_ip=env_info.endpoint,
                                                    env_name=env_info.name, user_id=user_id)
            else:
                # 不存在告警标记，设置告警标记
                EnvLinkStatusChecker.set_alarm_flag(env_id=env_id)
        # 修改状态为未连通
        resource_service.update_link_status(env_id_list=[env_id], link_status=LinkStatusEnum.Offline)

    @staticmethod
    def check_link_status(env_id: str, link_status: LinkStatusEnum):
        """
        检查受保护环境连通性
        :param env_id: 保护环境id
        :param link_status: 保护环境连接状态
        """
        try:
            env_info = resource_service.query_resource_by_id(env_id)
            if not env_info:
                return
            if link_status is LinkStatusEnum.Online:
                EnvLinkStatusChecker.handle_link_status_online(env_id, env_info)
            else:
                EnvLinkStatusChecker.handle_link_status_offline(env_id, env_info)
        except Exception:
            logger.exception(f"check link status error resource [{env_id}]")
        finally:
            pass


class ServiceInstanceManager(object):
    """
    vsphere ServiceInstance管理器，对ServiceInstance进行缓存管理，避免连接过多和新建连接耗时
    """
    _lock = threading.Lock()
    _instance_list = {}
    _interval = KEEP_CHECK_ALIVE_INTERVAL

    def __init__(self):
        self._keepalive_timer = threading.Timer(
            interval=self._interval, function=self._keepalive)
        self._keepalive_timer.start()

    @classmethod
    def _get_vspher_scan_param(cls, env_id):
        """
        将vspher资源解析成scanenvparam的形式 方便获取vspher接口服务实例
        """
        params = resource_service.query_resource_by_id(env_id).dict()
        params["password"] = SystemBaseClient.decrypt(params.get("password")).get("plaintext")
        return ScanEnvSchema.parse_obj(params)

    def _do_login(self, scan_env: ScanEnvSchema):
        service_instance, cert_name = VMwareDiscoveryService.login(scan_env)
        self.pub_service_instance(scan_env.uuid, service_instance)
        return service_instance, cert_name

    def get_service_instance(self, uuid):
        """
        获取vsphere管理接口的服务实例
        :param uuid: vCenter/ESX/ESXi的uuid
        :return: ServiceInstance
        """
        service_instance = self._instance_list.get(uuid)
        if service_instance is None:
            param = self._get_vspher_scan_param(uuid)
            service_instance, cert_name = VMwareDiscoveryService.login(param)
            self.pub_service_instance(uuid, service_instance)
            service_instance.RetrieveContent()
            clear(param.password)
            return service_instance

        try:
            content = service_instance.RetrieveContent()
            if content and content.sessionManager and content.sessionManager.currentSession:
                return service_instance
            else:
                self._instance_list.pop(uuid)
                return self.get_service_instance(uuid)
        except Exception as ex:
            logger.exception(f"vcenter[{uuid}] connection error")
            class_name = ex.__class__.__name__
            if class_name == 'timeout' or class_name == 'TypeError':
                raise EmeiStorBizException(ResourceErrorCodes.NETWORK_CONNECTION_TIMEDOUT,
                                           message="Network connection timed out.", retryable=True) from ex
            try:
                logger.info(f"Connect to VMware[uuid: {uuid}] failed, try to reconnect.")
                param = self._get_vspher_scan_param(uuid)
                service_instance, cert_name = VMwareDiscoveryService.login(param)
                self.pub_service_instance(uuid, service_instance)
                service_instance.RetrieveContent()
                clear(param.password)
                return service_instance
            except Exception as exception:
                logger.error(f"The VMware[uuid: {uuid}] connection failed.")
                raise EmeiStorBizException(ResourceErrorCodes.VMWARE_CONNECTION_FAILED,
                                           message="The VMware connection failed.", retryable=True) from exception
        finally:
            pass

    def pub_service_instance(self, uuid, service_instance):
        """
        将ServiceInstance放入管理器缓存起来，方便下次重用
        :param uuid: vCenter/ESX/ESXi的uuid
        :param service_instance: ServiceInstance
        :return:
        """
        self._instance_list[uuid] = service_instance

    def _keepalive(self):
        """
        使放入缓存中的serviceinstance保活
        :return:
        """
        for key in list(self._instance_list.keys()):
            self._check_alive(key)
        self._keepalive_timer = threading.Timer(
            interval=self._interval, function=self._keepalive)
        self._keepalive_timer.start()

    def _check_alive(self, key):
        logger.debug(f"Check alive,vcenter[{key}] check instance is alive start.")
        # 从本地缓存获取vcenter/esx对应的service_instance
        service_instance = self._instance_list.get(key)
        if not service_instance:
            # 若缓存的空数据，清楚掉本地缓存
            self._instance_list.pop(key)
            return
        try:
            content = service_instance.RetrieveContent()
            if not content.sessionManager or not content.sessionManager.currentSession:
                logger.warn(f"vcenter[{key}] current instance has expired, remove local cache.")
                self._instance_list.pop(key)
            EnvLinkStatusChecker.check_link_status(env_id=key, link_status=LinkStatusEnum.Online)
        except Exception:
            logger.exception("vcenter keep alive failed")
            try:
                logger.info(f"Connect to VMware[uuid: {key}] failed, try to reconnect.")
                param = self._get_vspher_scan_param(key)
                service_instance, cert_name = VMwareDiscoveryService.login(param)
                self.pub_service_instance(key, service_instance)
                service_instance.RetrieveContent()
                EnvLinkStatusChecker.check_link_status(env_id=key, link_status=LinkStatusEnum.Online)
                clear(param.password)
            except Exception:
                logger.error(f"The VMware[uuid: {key}] connection failed.")
                EnvLinkStatusChecker.check_link_status(env_id=key, link_status=LinkStatusEnum.Offline)
        finally:
            pass
        logger.debug(f"Check alive,vcenter[{key}] check instance is alive finish.")


service_instance_manager = ServiceInstanceManager()
