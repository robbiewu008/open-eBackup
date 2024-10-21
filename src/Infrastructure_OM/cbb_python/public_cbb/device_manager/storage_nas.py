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
#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import ipaddress
import json
import os
import re
import time

from public_cbb.device_manager.schemas.response import RequestResult, ResUserInGroup
from public_cbb.device_manager.sudo_oper import mount_nfs_by_root_account, shell_log_list_print, \
    delete_path_by_root_account, umount_by_root_account, mount_cifs_by_root_account, check_is_directory_sudo, \
    check_is_mount_sudo, clear_path_by_root_account, mount_fuse_by_root_account, umount_fuse_by_root_account
from public_cbb.communication.rest.https_request import HttpRequest
from public_cbb.exception.custom_exception import CustomException
from public_cbb.device_manager.storage_base_common import StorageBaseCommon
from public_cbb.log.logger import get_logger
from public_cbb.device_manager.device_info import (
    DeviceDetails, LogicPortInfo, NasShareInfo, NasSharedClientInfo, NasSnapshotInfo, RevertInfo, DtreeInfo,
    NasMountInfo, SmartQoSInfo, WindowsUserInfo, ReductionInfo, SystemInfo, StorageTaskInfo, ModifyFileSystemInfo,
    CifsIpControllerRule, OsaLivemountInfo
)
from public_cbb.device_manager.constants import (
    HostType, FILE_SYSTEM_SECTOR_IN_BYTES, NUM_1024, FILE_SYSTEM_TYPE, ReturnCode, NasShareType, RunningStatusEnum,
    ReadWriteStatusEnum, LogicPortRunningStatusEnum, LogicPortRoleEnum, NetPlaneTypeEnum, NfsPermissionEnum,
    NfsWriteModeEnum, NfsPermissionConstraintEnum, NfsRootPermissionConstraintEnum, CifsDomainTypeEnum,
    CifsPermissionEnum, ROLL_BACK_SPEED, MAX_ROLLBACK_RATE, DtreeParentTypeEnum, DtreeSecurityStyleEnum,
    StorageErrorCode, RollbackStatusEnum, SpaceAllocationType, QOS_NAME_MAX_LENGTH, PortVerification, NUM_50, NUM_99,
    DATA_TURBO_ROLE_ID, DATA_TURBO_SCOPE, DATA_TURBO_VSTOREID, DATA_TURBO_PROTOCOL, WORM_TYPE, WORM_UNIT, WORM_PERIOD,
    WORM_MIN_PROTECT_PERIOD, KERBEROS_5P_OPEN, FILE_SYSTEM_BANDWIDTH, MIN_PROTECT_TIME_UNIT, SpaceSelfAdjustingMode,
    AUTO_GROW_THRESHOLD_PERCENT, MAX_LOGIC_CAPACITY_FOR_FILESYSTEM, DEFAULT_ALARM_CAPACITY_THRESHOLD, DEFAULT_VSTOREID,
    DEPLOY_TYPE_GROUP_NO_REPLICATION, DEPLOY_TYPE_GROUP_NO_MIGRATING, CONVERGED_QOS_NAME_MAX_LENGTH
)
from public_cbb.config.global_config import get_settings
from public_cbb.security.anonym_utils.anonymity import Anonymity

log = get_logger()


class StorageNas(StorageBaseCommon):
    def __init__(self, device_info, nas_share_type=NasShareType.NFS):
        super().__init__(device_info)
        self._nas_share_type = nas_share_type

    @staticmethod
    def mount(mount_info: NasMountInfo):
        error_code, error_desc = StorageNas.validate_mount_path(mount_info.local_path)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        error_code, error_desc = StorageNas.validate_mount_path(mount_info.mount_path)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        try:
            ip_addr = mount_info.mount_ip
            ip_type = ipaddress.ip_address(mount_info.mount_ip)
            if isinstance(ip_type, ipaddress.IPv6Address):
                ip_addr = f'[{mount_info.mount_ip}]'
            log.debug(f'Mount path from {ip_addr}:{mount_info.mount_path} to {mount_info.local_path}')
            if mount_info.mount_type == NasShareType.NFS:
                return_code, error_desc = StorageNas.nfs_mount(ip_addr, mount_info)
                return return_code, error_desc
            elif mount_info.mount_type == NasShareType.CIFS:
                mount_src = f"//{ip_addr}/{mount_info.mount_path.lstrip('/')}"
                mount_point = mount_info.local_path.rstrip('/')
                if not mount_info.cifs_user_name or not mount_info.cifs_password:
                    log.error(f"Cifs mount user name or pass is null.")
                    return ReturnCode.FAILED, 'Cifs mount user name or pass is null.'
                out, err, return_code = mount_cifs_by_root_account(mount_point, mount_src, mount_info.cifs_user_name,
                                                                   mount_info.cifs_password)
                last_error_desc = shell_log_list_print(out)
                error_desc = err.decode("UTF-8")
                if return_code == ReturnCode.SUCCESS:
                    log.info(f'Mount cifs success, mount point:{mount_point}')
                    return ReturnCode.SUCCESS, ''
                if not error_desc:
                    error_desc = last_error_desc
                log.error(f"Fail to mount cifs, error code: {return_code}, error desc: {error_desc}")
                return return_code, error_desc
            elif mount_info.mount_type == NasShareType.FUSE:
                return_code, error_desc = StorageNas.fuse_mount(mount_info)
                return return_code, error_desc
            else:
                error_code, error_desc = ReturnCode.FAILED, f'The mount type:{mount_info.mount_type} is not supported.'
                log.error(f"Mount fuse fail, error code: {error_code}, error desc: {error_desc}")
                return error_code, error_desc
        except Exception as error:
            log.error(f"Mount fuse exception: {Anonymity.process(str(error))}")
            return ReturnCode.FAILED, str(error)

    @staticmethod
    def nfs_mount(ip_addr, mount_info):
        mount_src = f"{ip_addr}:/{mount_info.mount_path.lstrip('/')}"
        mount_point = mount_info.local_path.rstrip('/')
        out, err, return_code = mount_nfs_by_root_account(mount_point, mount_src)
        last_error_desc = shell_log_list_print(out)
        error_desc = err.decode("UTF-8")
        if return_code == ReturnCode.SUCCESS:
            log.info(f'Mount nas success, mount point:{mount_point}')
            return ReturnCode.SUCCESS, ''
        if not error_desc:
            error_desc = last_error_desc
        log.error(f"Fail to mount nas, error code: {return_code}, error desc: {error_desc}")
        return return_code, error_desc

    @staticmethod
    def fuse_mount(mount_info):
        source_id = mount_info.source_id
        mount_point = mount_info.mount_point.rstrip('/')
        out, err, return_code = mount_fuse_by_root_account(mount_point, source_id, mount_info.osad_ip_list,
                                                           mount_info.osad_auth_port,
                                                           mount_info.osad_server_port)
        last_error_desc = shell_log_list_print(out)
        error_desc = err.decode("UTF-8")
        if return_code == ReturnCode.SUCCESS:
            log.info(f'Mount fuse success, mount point:{mount_point}')
            return ReturnCode.SUCCESS, ''
        if not error_desc:
            error_desc = last_error_desc
        log.error(f"Fail to mount fuse, error code: {return_code}, error desc: {error_desc}")
        return return_code, error_desc

    @staticmethod
    def umount(mount_info):
        local_path = mount_info.local_path.rstrip('/')
        error_code, error_desc = StorageNas.validate_mount_path(local_path)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        try:
            if not check_is_directory_sudo(local_path):
                log.info(f'Path:{local_path} is not dir. No need to umount.')
                return ReturnCode.SUCCESS, ''
            log.debug(f'Umount {local_path}')
            if mount_info.mount_type == NasShareType.FUSE:
                out, err, return_code = umount_fuse_by_root_account(mount_info.mount_point)
            else:
                if not check_is_mount_sudo(local_path):
                    log.info(f'Path:{local_path} is not mount point. No need to umount.')
                    delete_path_by_root_account(local_path)
                    return ReturnCode.SUCCESS, ''
                out, err, return_code = umount_by_root_account(local_path)
            last_error_desc = shell_log_list_print(out)
            error_desc = err.decode("UTF-8")
            if error_desc or (return_code != ReturnCode.SUCCESS):
                if not error_desc:
                    error_desc = last_error_desc
                log.error(f"Fail to umount nas device, error code: {return_code}, error desc: {error_desc}")
                return return_code, error_desc
            else:
                log.info(f"umount nas, start to delete dir: {local_path}")
                delete_path_by_root_account(local_path)
            return ReturnCode.SUCCESS, ''
        except Exception as error:
            log.error(f"Umount exception: {Anonymity.process(str(error))}")
            return ReturnCode.FAILED, str(error)

    @staticmethod
    def remove_path_from_mount_path(delete_path, is_file=False):
        delete_path = delete_path.rstrip('/')
        validate_path = delete_path
        if is_file:
            validate_path = validate_path.strip().rsplit('/', 1)[0]
        error_code, error_desc = StorageNas.validate_mount_path(validate_path)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        try:
            log.debug(f'Start to delete path:{delete_path}')
            out, err, return_code = delete_path_by_root_account(delete_path)
            last_error_desc = shell_log_list_print(out)
            error_desc = err.decode("UTF-8")
            if error_desc or (return_code != ReturnCode.SUCCESS):
                if not error_desc:
                    error_desc = last_error_desc
                log.error(f"Fail to delete path, error code: {return_code}, error desc: {error_desc}")
                return return_code, error_desc
            return ReturnCode.SUCCESS, ''
        except Exception as error:
            log.error(f"Delete path exception: {Anonymity.process(str(error))}")
            return ReturnCode.FAILED, str(error)

    @staticmethod
    def clear_path_from_mount_path(clear_path, is_file=False):
        clear_path = clear_path.rstrip('/')
        validate_path = clear_path
        if is_file:
            validate_path = validate_path.strip().rsplit('/', 1)[0]
        error_code, error_desc = StorageNas.validate_mount_path(validate_path)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        try:
            log.debug(f'Start to clear path:{clear_path}')
            out, err, return_code = clear_path_by_root_account(clear_path)
            last_error_desc = shell_log_list_print(out)
            error_desc = err.decode("UTF-8")
            if error_desc or (return_code != ReturnCode.SUCCESS):
                if not error_desc:
                    error_desc = last_error_desc
                log.error(f"Fail to clear path, error code: {return_code}, error desc: {error_desc}")
                return return_code, error_desc
            return ReturnCode.SUCCESS, ''
        except Exception as error:
            log.error(f"Clear path exception: {Anonymity.process(str(error))}")
            return ReturnCode.FAILED, str(error)

    @staticmethod
    def validate_mount_path(path):
        if '.snapshot' in path:
            return ReturnCode.SUCCESS, ''

        # 字符串既不以 ../ 、./ 或 .\ 开头，也不包含这些子字符串。
        first_regex = r'^(?!.*(\.\./|\.\\|\.\/)).+$'
        # 不能为空，不能含有?*|;&$><`"\\!等字符
        regex = r'^[^?*|;&$><`"\\!]+$'
        if not re.match(first_regex, path) or not re.match(regex, path):
            error_desc = f'Illegal file mount path: {path}'
            log.error(f'Validate mount path fail, error desc: {error_desc}')
            return ReturnCode.FAILED, error_desc
        return ReturnCode.SUCCESS, ''

    @staticmethod
    def is_available_timestamp(timestamp: int):
        try:
            time.localtime(timestamp)
            return True
        except Exception as e:
            log.error(f'Timestamp: {timestamp} is not available timestamp, e:{Anonymity.process(str(e))}')
            return False

    @staticmethod
    def _get_backend_data_ip(ip_list: list, net_plane_type=NetPlaneTypeEnum.NAS_STORAGE_KUBERNETES):
        with open(get_settings().ANNOTATIONS_CONFIG_PATH, 'r') as f:
            for line in f.readlines():
                if not line.startswith(f'{net_plane_type}='):
                    continue
                ip_list.append(line.split('=')[-1].strip('\n\"'))
                break
        if not ip_list:
            raise Exception(f'Can not find {net_plane_type} in annotations.')
        log.debug(f'Read {net_plane_type} from annotations successfully. IP:{ip_list}')

    def set_share_type(self, nas_share_type: NasShareType):
        self._nas_share_type = nas_share_type
        return self._nas_share_type

    def get_share_type(self):
        return self._nas_share_type

    def query_storage_controller_info(self, controller_id_list: list):
        controller_id_list.clear()
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/controller'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            if rsp.get('data') and isinstance(rsp.get('data'), list):
                for controller in rsp.get('data'):
                    controller_id_list.append(controller.get('ID'))
                return ReturnCode.SUCCESS, ''
            else:
                error_code, error_desc = ReturnCode.FAILED, 'Storage controller info is null.'
                log.debug(f"Query storage controller info does not exist, desc: {error_desc}")
                return error_code, error_desc
        log.error(f"Query storage controller info fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def query_storage_pool_info(self, pool_id_list: list):
        pool_id_list.clear()
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/storagepools'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if rsp.get('data') and isinstance(rsp.get('data'), list):
            for pool in rsp.get('data'):
                pool_id_list.append(pool.get('ID'))
            return ReturnCode.SUCCESS, ''
        else:
            error_code, error_desc = ReturnCode.FAILED, 'Storage pool info is null.'
            log.debug(f"Query storage pool info does not exist, desc: {error_desc}")
            return error_code, error_desc

    def query_storage_pool_from_id(self, pool_id, pool_details):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/storagepool'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS and rsp.get('data') and isinstance(rsp.get('data'), list):
            for pool in rsp.get('data'):
                if pool.get('ID') == pool_id:
                    pool_details.total_capacity = int(pool.get('USERTOTALCAPACITY', 0))
                    return ReturnCode.SUCCESS, ''
            else:
                error_code, error_desc = ReturnCode.FAILED, 'Storage pool info is null.'
                log.debug(f"Query storage pool from id does not exist, desc: {error_desc}")
                return error_code, error_desc
        log.error(f"Query storage pool from id failed, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def query_storage_pool_parent_id(self, parent_id_list, storage_id):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/storagepools'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS and rsp.get('data') and isinstance(rsp.get('data'), list):
            for pool in rsp.get('data'):
                if storage_id and pool.get('ID') != storage_id:
                    continue
                else:
                    parent_id_list.append(pool.get('PARENTID'))
            return ReturnCode.SUCCESS, ''
        log.error(f"Query storage pool parent id failed, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def query_control_box_by_storage_pool_parent_id(self, parent_id, box_list):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/get_all_engine_diskInfo?disk_pool_id={parent_id}'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            disk_infos = rsp.get('data', {}).get('allEngineDiskInfo', '{}')
            if not disk_infos:
                return error_code, error_desc
            member_disk_infos = json.loads(disk_infos).get('memberDisk', [])  # 将字符串解析成字典再获取参数
            for member_disk_info in member_disk_infos:
                for member_disk_box in member_disk_info.keys():
                    box = re.search(r'(CTE)(\d+)', member_disk_box)
                    box_list.append(box.group(2)) if box else None
            return ReturnCode.SUCCESS, ''
        log.error(f"Query storage pool parent id failed, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def query_system_info(self, system_info: SystemInfo):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = '/system'

        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            if rsp.get('data'):
                system_info.sys_id = rsp.get('data').get('ID')
                system_info.product_mode = rsp.get('data').get('PRODUCTMODE')
                system_info.running_status = rsp.get('data').get('RUNNINGSTATUS')
                return ReturnCode.SUCCESS, ''
            else:
                error_code, error_desc = ReturnCode.FAILED, 'System info is null.'
                log.error(f"Query system info does not exist, desc: {error_desc}")
                return error_code, error_desc
        log.error(f"Query system info fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def create_global_secure_compliance_clock(self):
        req = HttpRequest()
        req.method = 'PUT'
        req.suffix = '/global_secure_compliance_clock'
        req.body = json.dumps({
            'wormGlobalClock': f'{round(time.time())}'
        })
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            log.info(f'Create global security compliance clock success')
        elif error_code == StorageErrorCode.GLOBAL_SECURE_COMPLIANCE_CLOCK_ALREADY_EXIST:
            log.info(f'Global clock already exist, do not need to recreate one')
            return ReturnCode.SUCCESS, 'Global clock already exist, do not need to recreate one'
        else:
            log.error(f'Create global clock failed')
        return error_code, error_desc

    def query_global_secure_compliance_clock(self):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = '/global_secure_compliance_clock'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            log.info(f'Found global security compliance clock success')
        elif error_code == StorageErrorCode.GLOBAL_SECURE_COMPLIANCE_CLOCK_NOT_EXIST:
            log.info(f'Global clock does not exist')
        else:
            log.error(f'Query global clock failed')
        return error_code, error_desc

    def check_is_migrating(self, pool_id, device_detail: DeviceDetails):
        req = HttpRequest()
        req.method = 'GET'
        # 从存储池获取硬盘域id
        req.suffix = f'/storagepool?pool_id={pool_id}'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            disk_pool_id = rsp.get('data').get('PARENTID', None)
            log.info(f"Query storage pool info succeed, parent disk pool id:{disk_pool_id}")
        else:
            log.error(f"Query disk pool info failed, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc

        # 唯一存储池
        req.suffix = f'/diskpool/{disk_pool_id}'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            if rsp.get('data'):
                device_detail.is_migrating = rsp.get('data').get('existShardMigrate', None)
                log.info(f"Query disk pool info succeed, is migrating: {device_detail.is_migrating}")
                return ReturnCode.SUCCESS, ''
            else:
                error_code, error_desc = ReturnCode.FAILED, 'Disk pool info is null.'
                log.error(f"Query disk pool info does not exist, desc: {error_desc}")
                return error_code, error_desc
        log.error(f"Query disk pool info failed, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def set_fs_worm(self, file_system_name):
        # pacific 文件系统的worm需要单独配置
        req = HttpRequest()
        req.method, req.suffix = 'POST', '/worm_policy'
        body = {}
        body['FSNAME'] = f'{file_system_name}'
        body['WORMTYPE'] = f'{WORM_TYPE}'
        body['MINPROTECTTIMEUNIT'] = MIN_PROTECT_TIME_UNIT
        body['MAXPROTECTTIMEUNIT'] = WORM_UNIT
        body['DEFPROTECTTIMEUNIT'] = MIN_PROTECT_TIME_UNIT
        body['WORMDEFPROTECTPERIOD'] = WORM_MIN_PROTECT_PERIOD
        body['WORMMAXPROTECTPERIOD'] = WORM_PERIOD
        body['WORMMINPROTECTPERIOD'] = WORM_MIN_PROTECT_PERIOD
        body['WORMAUTOLOCK'] = False
        req.body = json.dumps(body)
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        log.info(f"error_code: {error_code}, error_desc:{error_desc}")
        return error_code, error_desc

    def create_file_system(self, file_system_name, size_in_mega_bytes=None, dist_alg=True, owning_controller_id=None,
                           try_all_pool=True, capacity_threshold=None, is_worm_on=False, source_fs_name=None,
                           character_set=0, auto_grow_capacity=True, storage_info=None):
        if storage_info is None:
            storage_info = {}
        device_details = DeviceDetails()
        error_code, error_desc = self.query_file_system(file_system_name, device_details)
        if error_code == ReturnCode.SUCCESS:
            return ReturnCode.SUCCESS, ''
        pool_id_list = []
        if try_all_pool:
            error_code, error_desc = self.query_storage_pool_info(pool_id_list)
            if error_code != ReturnCode.SUCCESS:
                return error_code, error_desc
        else:
            pool_id_list.append(storage_info.get("storage_pool")) if storage_info.get("storage_pool") else pool_id_list
        # 有一个存储池上创建成功就退出
        for pool_id in pool_id_list:
            req = HttpRequest()
            req.method, req.suffix = 'POST', '/filesystem'
            body = self._build_create_fs_body(source_fs_name, file_system_name, pool_id, size_in_mega_bytes, dist_alg,
                                              owning_controller_id, capacity_threshold, is_worm_on, character_set,
                                              auto_grow_capacity)
            body["account_id"] = "0"
            req.body = json.dumps(body)
            rsp = self._send_request(req, lock_session=True)
            error_code, error_desc = self._check_response(rsp)
            if error_code == StorageErrorCode.FILE_SYSTEM_ALREADY_EXIST:    # !!!op和pacific错误码不一致!!!
                log.info(f"File system already exist, no need to create, file system name:{file_system_name}.")
                if not is_worm_on:
                    return ReturnCode.SUCCESS, ''
                error_code, error_desc = self.set_fs_worm(file_system_name)
                if error_code != ReturnCode.SUCCESS:
                    return ReturnCode.SUCCESS, ''
                return error_code, ''

            if error_code != ReturnCode.SUCCESS:
                # 文件系统创建失败，继续尝试使用下一个存储池进行创建
                log.error(f"Failed to create file system in pool:{pool_id}, error code:{error_code},error:{error_desc}")
                continue
            # pacific 配置worm需要另外调用接口
            if is_worm_on:
                error_code, error_desc = self.set_fs_worm(file_system_name)
                if error_code != ReturnCode.SUCCESS:
                    continue
            # 文件系统创建成功，等待状态就绪
            if not rsp.get('data') or not rsp.get('data').get('RUNNINGSTATUS') or not rsp.get('data').get('rwStatus'):
                error_code, error_desc = ReturnCode.FAILED, 'The file system data is null.'
                log.debug(f"Create file system, rsp data not exist, desc: {error_desc}")
                return error_code, error_desc
            device_details.running_status = rsp.get('data').get('RUNNINGSTATUS')
            device_details.read_write_status = rsp.get('data').get('rwStatus')
            if int(device_details.running_status) == RunningStatusEnum.ONLINE \
                    and int(device_details.read_write_status) == ReadWriteStatusEnum.READ_WRITE:
                return ReturnCode.SUCCESS, ''
            else:
                return self.wait_file_system_status_ready(file_system_name, device_details)
        # 所有存储池均创建失败
        error_code, error_desc = ReturnCode.FAILED, 'Failed try all pool to create file system.'
        log.error(f"Error_desc:{error_desc}")
        return error_code, error_desc

    def query_file_system_by_id(self, file_system_id, device_details: DeviceDetails):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/filesystem/{file_system_id}'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            if rsp.get('data') and isinstance(rsp.get('data'), dict):
                device_details.worm_type = int(rsp.get('data').get('WORMTYPE', 0))
                device_details.security_style = int(rsp.get('data').get('securityStyle', 0))
                device_details.alloc_type = int(rsp.get('data').get('ALLOCTYPE', 0))
                device_details.parent_id = str(rsp.get('data').get('PARENTID', 0))
                device_details.is_show_snap_dir = True if rsp.get('data').get('ISSHOWSNAPDIR') == 'true' else False
                device_details.total_capacity = \
                    int(rsp.get('data').get('CAPACITY', 0))
                device_details.running_status = rsp.get('data').get('RUNNINGSTATUS')
                device_details.read_write_status = rsp.get('data').get('rwStatus')
                used_capacity = rsp.get('data').get('allocatedPoolQuota', 0)
                device_details.used_capacity = int(used_capacity) if used_capacity else 0
                device_details.remote_replication_ids = rsp.get('data').get('REMOTEREPLICATIONIDS')
                device_details.total_saved_capacity = \
                    int(rsp.get('data').get('TOTALSAVEDCAPACITY', 0))
                device_details.capacity_threshold = int(rsp.get('data').get('CAPACITYTHRESHOLD', 0))
                device_details.used_capacity_ratio = int(rsp.get('data').get('AVAILABLEANDALLOCCAPACITYRATIO', 0))
                device_details.sector_size = int(rsp.get('data').get('SECTORSIZE', 0))
                device_details.dist_alg = int(rsp.get('data').get('distAlg', 0))
                device_details.skip_affinity_mode_count = int(rsp.get('data').get('skipAffModeCount', 0))
                device_details.unix_permission = int(rsp.get('data').get('unixPermissions', 0))
                device_details.device_id = rsp.get('data').get('ID')
                device_details.device_name = rsp.get('data').get('NAME')
                device_details.owning_controller = rsp.get('data').get('OWNINGCONTROLLER')
                device_details.compress_enable = True if rsp.get('data').get('ENABLECOMPRESSION') == 'true' else False
                device_details.compress_algorithm = int(rsp.get('data').get('COMPRESSION', 0))
                device_details.dedup_enable = (True if rsp.get('data').get('ENABLEDEDUP') == 'true' else False)
                return ReturnCode.SUCCESS, ''
            else:
                error_code, error_desc = ReturnCode.NOT_EXIST, 'The file system data is null.'
                log.debug(f"Query file system not exist, desc: {error_desc}")
                return error_code, error_desc
        log.error(f"Query file system fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def query_file_system(self, file_system_name, device_details: DeviceDetails):
        req = HttpRequest()
        req.method = 'GET'

        req.suffix = f'/filesystem?fs_name={file_system_name}'

        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            if rsp.get('data') and isinstance(rsp.get('data'), list) and \
                    rsp.get('data')[0].get('NAME') == file_system_name:
                device_details.device_id = rsp.get('data')[0].get('ID')
                device_details.device_name = rsp.get('data')[0].get('NAME')
                device_details.owning_controller = rsp.get('data')[0].get('OWNINGCONTROLLER')
                device_details.compress_enable = (True if rsp.get('data')[0].get('ENABLECOMPRESSION') == 'true'
                                                  else False)
                device_details.compress_algorithm = int(rsp.get('data')[0].get('COMPRESSION', 0))
                device_details.dedup_enable = (True if rsp.get('data')[0].get('ENABLEDEDUP') == 'true' else False)
                device_details.total_capacity = \
                    int(rsp.get('data')[0].get('CAPACITY', 0)) * FILE_SYSTEM_SECTOR_IN_BYTES
                device_details.running_status = rsp.get('data')[0].get('RUNNINGSTATUS')
                device_details.read_write_status = rsp.get('data')[0].get('rwStatus')
                used_capacity = rsp.get('data')[0].get('allocatedPoolQuota', 0)
                device_details.used_capacity = int(used_capacity) * FILE_SYSTEM_SECTOR_IN_BYTES if used_capacity else 0
                device_details.remote_replication_ids = rsp.get('data')[0].get('REMOTEREPLICATIONIDS')
                device_details.total_saved_capacity = \
                    int(rsp.get('data')[0].get('TOTALSAVEDCAPACITY', 0)) * FILE_SYSTEM_SECTOR_IN_BYTES
                device_details.capacity_threshold = int(rsp.get('data')[0].get('CAPACITYTHRESHOLD', 0))
                device_details.used_capacity_ratio = int(rsp.get('data')[0].get('AVAILABLEANDALLOCCAPACITYRATIO', 0))
                device_details.worm_type = int(rsp.get('data')[0].get('WORMTYPE', 0))
                device_details.security_style = int(rsp.get('data')[0].get('securityStyle', 0))
                device_details.alloc_type = int(rsp.get('data')[0].get('ALLOCTYPE', 0))
                device_details.parent_id = str(rsp.get('data')[0].get('PARENTID', 0))
                device_details.is_show_snap_dir = True if rsp.get('data')[0].get('ISSHOWSNAPDIR') == 'true' else False
                device_details.sector_size = int(rsp.get('data')[0].get('SECTORSIZE', 0))
                device_details.dist_alg = int(rsp.get('data')[0].get('distAlg', 0))
                device_details.skip_affinity_mode_count = int(rsp.get('data')[0].get('skipAffModeCount', 0))
                device_details.unix_permission = int(rsp.get('data')[0].get('unixPermissions', 0))
                device_details.io_class_id = str(rsp.get('data')[0].get('IOCLASSID', ''))
                device_details.space_self_adjusting_mode = int(rsp.get('data')[0].get('SPACESELFADJUSTINGMODE', 0))
                device_details.max_auto_size = int(rsp.get('data')[0].get('MAXAUTOSIZE', 0))
                device_details.auto_grow_threshold_percent = int(rsp.get('data')[0].get('AUTOGROWTHRESHOLDPERCENT', 0))
                device_details.parent_file_system_id = str(rsp.get('data')[0].get('PARENTFILESYSTEMID', ''))

                return ReturnCode.SUCCESS, ''
            else:
                error_code, error_desc = ReturnCode.NOT_EXIST, 'The file system data is null.'
                log.debug(f"Query file system not exist, desc: {error_desc}")
                return error_code, error_desc
        log.error(f"Query file system fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def get_file_system_reduction_info(self, file_system_id, reduction_info: ReductionInfo):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/filesystem/get_fs_reduction_info?ID={file_system_id}'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Get file system reduction info fail, error code:{error_code}, error desc:{error_desc}")
            return error_code, error_desc
        data = rsp.get('data', None)
        if not data:
            error_desc = 'Query file system reduction info not exist, desc: The data is null.'
            log.error(f"Error_desc:{error_desc}")
            return ReturnCode.FAILED, error_desc
        reduction_info.total_write_capacity = int(data.get('totalWriteCapacity', 0))
        reduction_info.reduction_ratio = data.get('reductionRatio')
        reduction_info.capacity_befor_reduction = int(data.get('capacityBeforReduction', 0))
        reduction_info.capacity_after_reduction = int(data.get('capacityAfterReduction', 0))
        reduction_info.excl_cap_befor_red = int(data.get('exclCapBeforRed', 0))
        reduction_info.excl_cap_after_red = int(data.get('exclCapAfterRed', 0))
        reduction_info.incompressible_cap = int(data.get('incompressibleCap', 0))
        reduction_info.shared_cap_ratio = data.get('sharedCapRatio')
        reduction_info.excl_cap_ratio = data.get('exclCapRatio')
        reduction_info.incompressible_ratio = data.get('incompressibleRatio')
        return ReturnCode.SUCCESS, ''

    def modify_file_system_info(self, modify_info: ModifyFileSystemInfo):
        log.info(f"Start to change file system info, file_system_id:{modify_info.file_system_id}, "
                 f"size:{modify_info.size_in_mega_bytes} MB, capacity_threshold:{modify_info.capacity_threshold}")
        req = HttpRequest()
        req.method = 'PUT'
        req.suffix = f'/filesystem/{modify_info.file_system_id}'
        body = {}
        if modify_info.size_in_mega_bytes:
            capacity = NUM_1024 if modify_info.size_in_mega_bytes < NUM_1024 else modify_info.size_in_mega_bytes
            body['CAPACITY'] = f'{int(capacity * NUM_1024 * NUM_1024 / FILE_SYSTEM_SECTOR_IN_BYTES)}'
        if modify_info.capacity_threshold and NUM_50 <= modify_info.capacity_threshold <= NUM_99:
            body['CAPACITYTHRESHOLD'] = f'{modify_info.capacity_threshold}'
        if modify_info.auto_grow_capacity:
            body['SPACESELFADJUSTINGMODE'] = SpaceSelfAdjustingMode.AUTOMATIC_CAPACITY_EXPANSION
            body['AUTOGROWTHRESHOLDPERCENT'] = AUTO_GROW_THRESHOLD_PERCENT
            body['MAXAUTOSIZE'] = MAX_LOGIC_CAPACITY_FOR_FILESYSTEM
        if modify_info.dist_alg:
            body['distAlg'] = modify_info.dist_alg
        if modify_info.security_style:
            body['securityStyle'] = modify_info.security_style
        body['ENABLECOMPRESSION'] = modify_info.compress_enable
        req.body = json.dumps(body)
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Modify file system info failed, error code:{error_code}, error desc:{error_desc}")
        return error_code, error_desc

    def delete_file_system(self, file_system_name):
        device_details = DeviceDetails()
        error_code, error_desc = self.query_file_system(file_system_name, device_details)
        if error_code == ReturnCode.NOT_EXIST:
            return ReturnCode.SUCCESS, ''
        shared_info = NasShareInfo()
        error_code, error_desc = self.query_share_info(device_details.device_id, shared_info)
        if error_code != ReturnCode.NOT_EXIST and error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        if error_code == ReturnCode.SUCCESS:
            error_code, error_desc = self.delete_share(shared_info.share_id)
            if error_code != ReturnCode.SUCCESS:
                return error_code, error_desc
        return self.delete_file_system_by_id(device_details.device_id)

    def delete_file_system_by_id(self, file_system_id):
        req = HttpRequest()
        req.method = 'DELETE'
        req.suffix = f'/filesystem/{file_system_id}'
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == StorageErrorCode.FILE_SYSTEM_ID_NOT_EXIST:
            log.debug(f"File system not exist, no need to delete, file_system_id:{file_system_id}")
            return ReturnCode.SUCCESS, ''
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Delete file system by id fail, error code:{error_code}, error desc:{error_desc}")
        return error_code, error_desc

    def query_service_host_logic_port(self, logic_port_list: list):
        logic_port_list.clear()
        lif_info = {}
        error_code, error_desc = self.query_logic_port_info(lif_info)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        error_code, error_desc = self._filter_logic_port(lif_info, logic_port_list)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        return ReturnCode.SUCCESS, ''

    def query_logic_port_info(self, lif_info: dict):
        lif_info.clear()
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = '/lif'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            if not rsp.get('data'):
                error_desc = 'Query logic port not exist, desc: The lif data is null.'
                log.debug(f"{error_desc}")
                return ReturnCode.FAILED, error_desc
            else:
                lif_info.update(rsp)
                return ReturnCode.SUCCESS, ''
        log.error(f"Query logic port fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    # 一体机环境，挂载文件系统到本地时，查询A8000的可挂载ip
    def query_service_host_inner(self, ip_list: list, net_plane_type=NetPlaneTypeEnum.NAS_STORAGE_KUBERNETES):
        ip_list.clear()
        try:
            self._get_backend_data_ip(ip_list, net_plane_type)
            return ReturnCode.SUCCESS, ''
        except Exception as e:
            log.error(f'Read net_plane_type:{net_plane_type} from annotations failed. Ex:{Anonymity.process(str(e))}')
            return ReturnCode.FAILED, str(e)

    # 创建根文件系统共享
    def create_share(self, file_system_name, file_system_id, share_name=None, is_ip_sec_open=False,
                     file_handle_byte_alignment=False):
        shared_info = NasShareInfo()
        error_code, error_desc = self.query_share_info(file_system_id, shared_info, share_name)
        if error_code == ReturnCode.SUCCESS:
            result = self._check_and_update_share_info(is_ip_sec_open, file_handle_byte_alignment, shared_info)
            if not result:
                log.error(f'Failed to update share info, file system name: {file_system_name}, '
                          f'file_system_id:{file_system_id}, share_name{share_name}')
            return ReturnCode.SUCCESS, ''
        req = HttpRequest()
        req.method = 'POST'
        if self._nas_share_type == NasShareType.NFS:
            req.suffix = '/NFSHARE'
            tmp_req_body = {
                'SHAREPATH': f'/{file_system_name}/',
                'FSID': f'{file_system_id}'
            }
            if file_handle_byte_alignment:
                tmp_req_body["fileHandleByteAlignmentSwitch"] = True
            req.body = json.dumps(tmp_req_body)
        elif self._nas_share_type == NasShareType.CIFS:
            req.suffix = '/CIFSHARE'
            req.body = json.dumps({
                'SHAREPATH': f'/{file_system_name}/',
                'NAME': f'{share_name}' if share_name else f'{file_system_name}',
                'FSID': f'{file_system_id}',
                'smb3EncryptionEnable': is_ip_sec_open,
                'unencryptedAccess': True
            })
        else:
            error_code, error_desc = ReturnCode.FAILED, f'The protocol {self._nas_share_type.name} is not supported.'
            log.error(f"Create share fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc

        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code in {StorageErrorCode.NFS_SHARE_ALREADY_EXIST, StorageErrorCode.NFS_ALIAS_EXIST,
                          StorageErrorCode.CIFS_SHARE_ALREADY_EXIST}:
            log.debug(f"File system share already exist, no need to create, file_system_name:{file_system_name}, "
                      f"share_name:{share_name}")
            return ReturnCode.SUCCESS, ''
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Create share fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def query_data_turbo_share_auth(self, username, share_id):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = '/DATATURBO_SHARE_AUTH'
        req.body = json.dumps({
            'vstoreId': f'{DATA_TURBO_VSTOREID}'
        })
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f'Failed to query data turbo share auth, error code: {error_code}, error desc: {error_desc}')
            return error_code, error_desc
        found = False
        for auth in rsp.get('data', []):
            if username == auth['userName'] and share_id == auth['shareId']:
                found = True
        if not found:
            log.info(f'Failed to find data turbo share auth, user: {username}, share_id: {share_id}')
            return ReturnCode.FAILED, 'Failed to find data turbo share auth'
        return error_code, error_desc

    def query_storage_task_status(self, task_info: StorageTaskInfo):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/api/v2/task/{task_info.task_id}'
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f'Failed to query storage status, error code: {error_code}, error desc: {error_desc}')
            return error_code, error_desc
        task_info.task_status = rsp.get('data', {}).get('taskStatus', '')
        no_need_wait_status = ['success', 'fail']
        if task_info.task_status not in no_need_wait_status:
            time.sleep(5)
            log.info(f'Wait for the task to be executed for 5 seconds.')
        return error_code, error_desc

    # 创建data turbo 系统共享
    def create_data_turbo_auth(self, username, share_id):
        req = HttpRequest()
        req.method = 'POST'
        req.body = json.dumps({
            'userName': username,
            'permission': '1',
            'vstoreId': str(DATA_TURBO_VSTOREID),
            'shareId': share_id
        })
        req.suffix = '/api/v2/task/add_dataturbo_auth'
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == StorageErrorCode.DATA_TURBO_SHARE_PERMISSION_ALREADY_EXIST:
            return ReturnCode.SUCCESS, error_desc
        storage_task_info = StorageTaskInfo()
        storage_task_info.task_id = rsp.get('data', {}).get('taskId')
        if not storage_task_info.task_id:
            return error_code, error_desc
        if error_code == ReturnCode.SUCCESS:
            return self.query_storage_task_status(storage_task_info)
        else:
            return error_code, error_desc

    def get_data_turbo_auth(self, file_system_and_dtree_path):
        req = HttpRequest()
        req.method = 'GET'

        req.suffix = f'/dataturbo_share_auth?filter_info={file_system_and_dtree_path}'

        req.body = json.dumps({
            'vstoreId': f'{DATA_TURBO_VSTOREID}'
        })
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f'Failed to query data turbo share auth, error code: {error_code}, error desc: {error_desc}')
            return error_code, error_desc, rsp
        return error_code, error_desc, rsp

    def delete_dataturbo_share_auth(self, user_name, file_system_and_dtree_path):
        error_code, error_desc, rsp = self.get_data_turbo_auth(file_system_and_dtree_path)
        if error_code != ReturnCode.SUCCESS:
            log.error(f'Failed to delete dt auth since can not get the auth list.  file_system_and_dtree_path: '
                      f'{file_system_and_dtree_path}')
            return ReturnCode.FAILED, ''
        auth_id = None
        for auth in rsp.get('data', []):
            if auth['sharePath'] == file_system_and_dtree_path and auth['userName'] == user_name:
                auth_id = auth['ID']
                log.info(f'Found the file_system_and_dtree_path:{file_system_and_dtree_path} '
                         f'auth with id: {auth_id}')
                break
        if auth_id is None:
            log.info(f'Did not find the auth with file/dtree path: {file_system_and_dtree_path},'
                     f'do not need to delete auth')
            return ReturnCode.SUCCESS, ''
        req = HttpRequest()
        req.method = 'DELETE'
        req.suffix = f'/DATATURBO_SHARE_AUTH/{auth_id}?userName={user_name}'
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Delete data turbo share auth fail, file_system_and_dtree_path: {file_system_and_dtree_path}"
                      f"error code:{error_code}, error desc:{error_desc}")
        return error_code, error_desc

    def create_data_turbo_share(self, file_system_name, file_system_id, share_name=None):
        req = HttpRequest()
        req.method = 'POST'
        req.suffix = '/DATATURBO_SHARE'
        req.body = json.dumps({
            'sharePath': f'/{file_system_name}/',
            'fileHandleByteAlignmentSwitch': True,
            'FSID': f'{file_system_id}'
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == StorageErrorCode.DATA_TURBO_SHARE_ALREADY_EXIST:
            log.info(f"Data turbo share already exist, file_system_name:{file_system_name},"
                     f"share_name:{share_name}, error code: {error_code}, error desc: {error_desc}")
            error_code, error_desc, share_id = self.query_data_turbo_share_info(file_system_name)
            return error_code, error_desc, share_id
        elif error_code == ReturnCode.SUCCESS:
            share_id = rsp.get('data', {'ID': -1}).get('ID', -1)
            return error_code, error_desc, share_id
        else:
            return error_code, error_desc, -1

    def query_data_turbo_share_info(self, file_system_name, dtree_path=None):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/DATATURBO_SHARE?fsName={file_system_name}'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Query data turbo share info fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc, -1
        if dtree_path is not None:
            file_system_name = dtree_path
        else:
            file_system_name = f'/{file_system_name}/'
        for data in rsp.get('data'):
            if data['sharePath'] == f'{file_system_name}':
                log.info(f'Query data turbo share info Succeed')
                return error_code, error_desc, data.get('ID', -1)
        log.info(f'Failed to find expeted data turbo share info with file_system_name:{file_system_name}')
        return StorageErrorCode.DATA_TURBO_SHARE_NOT_EXIST, '', -1

    def create_data_turbo_share_by_dtree_path(self, file_system_name, dtree_path):
        """
        :param file_system_name: 根文件系统名字，如 file_system_name="FS"
        :param dtree_path: 需要共享的dtree全路径，如 dtree_path="/FS/dtree"
        :return:
        """
        device_details = DeviceDetails()
        error_code, error_desc = self.query_file_system(file_system_name, device_details)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc, -1
        req = HttpRequest()
        req.method = 'POST'
        req.suffix = '/DATATURBO_SHARE'
        req.body = json.dumps({
            'sharePath': f'{dtree_path}',
            'fileHandleByteAlignmentSwitch': True,
            'FSID': f'{device_details.device_id}'
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == StorageErrorCode.DATA_TURBO_SHARE_ALREADY_EXIST:
            log.info(f"Dtree data turbo share already exist, file_system_name:{file_system_name},"
                     f" error code: {error_code}, error desc: {error_desc}")
            error_code, error_desc, share_id = self.query_data_turbo_share_info(file_system_name, dtree_path)
            return error_code, error_desc, share_id
        elif error_code == ReturnCode.SUCCESS:
            share_id = rsp.get('data', {'ID': -1}).get('ID', -1)
            log.info(f"Create Dtree data turbo share succeed, file_system_name:{file_system_name},"
                     f" error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc, share_id
        else:
            log.error(f"Failed to create Dtree data turbo share succeed, file_system_name:{file_system_name},"
                      f" error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc, -1

    # 查询根文件系统共享信息
    def query_share_info(self, file_system_id, shared_info: NasShareInfo, shared_name=None):
        req = HttpRequest()
        req.method = 'GET'
        if self._nas_share_type == NasShareType.NFS:
            req.suffix = '/NFSHARE'
            tmp_req_body = {
                'FSID': f'{file_system_id}'
            }
            req.body = json.dumps(tmp_req_body)
        elif self._nas_share_type == NasShareType.CIFS:
            req.suffix = '/CIFSHARE'
            tmp_req_body = {
                'FSID': f'{file_system_id}',
            }
            req.body = json.dumps(tmp_req_body)
        else:
            error_code, error_desc = ReturnCode.FAILED, f'The protocol {self._nas_share_type.name} is not supported.'
            log.error(f"Query share fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Query share fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        if not rsp.get('data') or not isinstance(rsp.get('data'), list):
            error_code, error_desc = ReturnCode.NOT_EXIST, 'The share info data is null.'
            log.debug(f"Query share info not exist, share_name:{shared_name}, desc: {error_desc}.")
            return error_code, error_desc
        if shared_name:
            for shared_tmp in rsp.get('data'):
                shared_tmp_name = {shared_tmp.get('NAME'), shared_tmp.get('NAME').replace('\\', '')}
                if shared_name in shared_tmp_name and self._nas_share_type == NasShareType.CIFS:
                    shared_info.smb3 = shared_tmp.get('smb3EncryptionEnable')
                if shared_name in shared_tmp_name:
                    shared_info.share_id = shared_tmp.get('ID')
                    shared_info.share_name = shared_tmp.get('NAME').replace('\\', '')
                    shared_info.share_path = shared_tmp.get('SHAREPATH').replace('\\', '')
                    shared_info.fs_id = shared_tmp.get('FSID')
                    shared_info.file_handle_byte_alignment = shared_tmp.get('fileHandleByteAlignmentSwitch', False)
                    return ReturnCode.SUCCESS, ''
            error_code, error_desc = ReturnCode.NOT_EXIST, f'The share info:{shared_name} not found.'
            log.debug(f"Query share info not exist, desc: {error_desc}")
            return error_code, error_desc
        if self._bind_share_info(rsp, shared_info):
            return ReturnCode.SUCCESS, ''
        return ReturnCode.NOT_EXIST, 'The share info not found.'

    def update_share_info(self, shared_info):
        req = HttpRequest()
        req.method = 'PUT'
        if self._nas_share_type == NasShareType.CIFS:
            req.suffix = f'/CIFSHARE/{shared_info.share_id}'
            req.body = json.dumps({
                'smb3EncryptionEnable': shared_info.smb3
            })
        else:
            req.suffix = f'/NFSHARE/{shared_info.share_id}'
            req.body = json.dumps({
                'fileHandleByteAlignmentSwitch': shared_info.file_handle_byte_alignment
            })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            log.info(f'Update share: {shared_info.share_id} succeed')
            return error_code, error_desc
        else:
            log.error(f"Update share: {shared_info.share_id} failed")
            return error_code, error_desc

    # 创建文件系统下指定Dtree的共享
    def create_share_by_dtree_path(self, file_system_name, dtree_path, share_name=None, is_ip_sec_open=False,
                                   file_handle_byte_alignment=False):
        """
        :param file_system_name: 根文件系统名字，如 file_system_name="FS"
        :param dtree_path: 需要共享的dtree全路径，如 dtree_path="/FS/dtree"
        :param share_name: 共享名称
        :param is_ip_sec_open: 链路加密开关
        :param file_handle_byte_alignment: 四字节对齐开关
        :return:
        """
        device_details = DeviceDetails()
        error_code, error_desc = self.query_file_system(file_system_name, device_details)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        shared_info = NasShareInfo()
        error_code, error_desc = self.query_share_info_by_dtree_path(device_details.device_id, dtree_path,
                                                                     shared_info, share_name)
        if error_code == ReturnCode.SUCCESS:
            result = self._check_and_update_share_info(is_ip_sec_open, file_handle_byte_alignment, shared_info)
            if not result:
                log.error(f'Failed to update share info, file system name: {file_system_name}, share_name{share_name}')
            return ReturnCode.SUCCESS, ''
        req = HttpRequest()
        req.method = 'POST'
        if self._nas_share_type == NasShareType.NFS:
            req.suffix = '/NFSHARE'
            tmp_req_body = {
                'SHAREPATH': f'{dtree_path}',
                'FSID': f'{device_details.device_id}'
            }
            if file_handle_byte_alignment:
                tmp_req_body["fileHandleByteAlignmentSwitch"] = True
            req.body = json.dumps(tmp_req_body)
        elif self._nas_share_type == NasShareType.CIFS:
            req.suffix = '/CIFSHARE'
            req.body = json.dumps({
                'SHAREPATH': f'{dtree_path}',
                'NAME': f'{share_name}' if share_name else f'{dtree_path.rsplit("/", 1)[-1]}',
                'FSID': f'{device_details.device_id}',
                'smb3EncryptionEnable': is_ip_sec_open,
                'unencryptedAccess': True
            })
        else:
            error_code, error_desc = ReturnCode.FAILED, f'The protocol {self._nas_share_type.name} is not supported.'
            log.error(f"Create share by dtree path fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc

        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS or error_code in {StorageErrorCode.NFS_SHARE_ALREADY_EXIST,
                                                              StorageErrorCode.CIFS_SHARE_ALREADY_EXIST,
                                                              StorageErrorCode.NFS_ALIAS_EXIST}:
            log.debug(f"Create share by dtree path success, error code: {error_code}, error desc: {error_desc}")
            return ReturnCode.SUCCESS, ''
        else:
            log.error(f"Create share by dtree path fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc

    # 查询文件系统下指定Dtree的共享信息
    def query_share_info_by_dtree_path(self, file_system_id, dtree_path, shared_info: NasShareInfo, share_name=None):
        """
        :param file_system_id: 根文件系统的id
        :param dtree_path: 需要查询共享的dtree全路径，如 dtree_path="/FS/dtree"
        :param shared_info: 返回值（share_id：用于添加白名单时使用）
        :param share_name: 共享名称
        :return:
        """
        req = HttpRequest()
        req.method = 'GET'
        if self._nas_share_type == NasShareType.NFS:
            req.suffix = '/NFSHARE'
            tmp_req_body = {'FSID': f'{file_system_id}'}
        elif self._nas_share_type == NasShareType.CIFS:
            req.suffix = '/CIFSHARE'
            tmp_req_body = {'FSID': f'{file_system_id}'}
        else:
            error_code, error_desc = ReturnCode.FAILED, f'The protocol {self._nas_share_type.name} is not supported.'
            log.error(f"Query share by dtree path fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        req.body = json.dumps(tmp_req_body)
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Query share by dtree path fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        if not rsp.get('data') or not isinstance(rsp.get('data'), list):
            error_code, error_desc = ReturnCode.NOT_EXIST, 'The share path data is null.'
            log.debug(f"Query share info, desc: {error_desc}")
            return error_code, error_desc
        if share_name:
            for shared_tmp in rsp.get('data'):
                shared_tmp_name = {shared_tmp.get('NAME'), shared_tmp.get('NAME').replace('\\', '')}
                if share_name in shared_tmp_name and self._nas_share_type == NasShareType.CIFS:
                    shared_info.smb3 = shared_tmp.get('smb3EncryptionEnable')
                if share_name in shared_tmp_name:
                    shared_info.share_id = shared_tmp.get('ID')
                    shared_info.share_name = shared_tmp.get('NAME').replace('\\', '')
                    shared_info.share_path = shared_tmp.get('SHAREPATH').replace('\\', '')
                    shared_info.fs_id = shared_tmp.get('FSID')
                    shared_info.file_handle_byte_alignment = shared_tmp.get('fileHandleByteAlignmentSwitch', False)
                    return ReturnCode.SUCCESS, ''
            error_code, error_desc = ReturnCode.NOT_EXIST, f'The share info:{share_name} not found.'
            log.debug(f"Query share info not exist, desc: {error_desc}")
            return error_code, error_desc
        for var in rsp.get('data'):
            rsp_share_path = {var.get('SHAREPATH'), var.get('SHAREPATH').replace('\\', '')}
            if dtree_path in rsp_share_path and self._nas_share_type == NasShareType.CIFS:
                shared_info.smb3 = var.get('smb3EncryptionEnable')
            if dtree_path in rsp_share_path:
                shared_info.share_id = var.get('ID')
                shared_info.share_name = var.get('NAME').replace('\\', '')
                shared_info.share_path = var.get('SHAREPATH').replace('\\', '')
                shared_info.fs_id = var.get('FSID')
                return ReturnCode.SUCCESS, ''
        error_code, error_desc = ReturnCode.NOT_EXIST, f'The share path "{dtree_path}" is not found.'
        log.debug(f"Query share info, desc: {error_desc}")
        return error_code, error_desc

    def delete_data_turbo_share(self, share_id):
        req = HttpRequest()
        req.method = 'DELETE'
        req.suffix = f'/DATATURBO_SHARE/{share_id}'
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == StorageErrorCode.DATA_TURBO_SHARE_DELETE_NOT_EXIST:
            log.debug(f"Data turbo share not exist, do not need to delete, share_id:{share_id}.")
            return ReturnCode.SUCCESS, ''
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Delete data turbo share fail, error code:{error_code}, error desc:{error_desc}")
        return error_code, error_desc

    def delete_share(self, share_id):
        req = HttpRequest()
        req.method = 'DELETE'
        if self._nas_share_type == NasShareType.NFS:
            req.suffix = f'/NFSHARE/{share_id}'
        elif self._nas_share_type == NasShareType.CIFS:
            req.suffix = f'/CIFSHARE/{share_id}'
        else:
            error_code, error_desc = ReturnCode.FAILED, f'The protocol:{self._nas_share_type.name} is not supported.'
            log.error(f"Query share fail, error code:{error_code}, error desc:{error_desc}")
            return error_code, error_desc
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == StorageErrorCode.FILE_SYSTEM_NOT_EXIST:
            log.debug(f"Share no exist, no need to delete, share_id:{share_id}.")
            return ReturnCode.SUCCESS, ''
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Delete share fail, error code:{error_code}, error desc:{error_desc}")
        return error_code, error_desc

    def add_share_client(self, share_id, user_name_or_ip, related_parameters=None):
        """
        :param share_id: Generated by create_share(), the value can be obtained through query_share_info().
        :param user_name_or_ip: if self._nas_share_type=CIFS use 'user name',
                                else if self._nas_share_type=NFS use 'ip'!
        :param related_parameters: dict{}
        """
        if not user_name_or_ip:
            error_code, error_desc = ReturnCode.FAILED, f'Input parameter user_name_or_ip is null!'
            log.error(f"Add share client fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        # 查询指定共享share_id下的白名单信息
        share_client_info_list = []
        error_code, error_desc = self.query_share_client(share_id, share_client_info_list)
        current_client_name_list = []
        if error_code == ReturnCode.SUCCESS:
            # 白名单中存在客户端，判断本次添加的客户端是否在该白名单中
            for share_client_info in share_client_info_list:
                current_client_name_list.append(share_client_info.name)
            if user_name_or_ip in current_client_name_list:
                # Share client already exist
                log.debug(f"Share client already exist, user_name_or_ip:{user_name_or_ip}, "
                          f"current_client_name_list:{current_client_name_list}")
                return ReturnCode.SUCCESS, ''
        log.info(f"Start to add share client, share_id:{share_id}, user_name_or_ip:{user_name_or_ip}, "
                 f"current_client_name_list:{current_client_name_list}")
        req = HttpRequest()
        req.method = 'POST'
        body_dict = {}
        if self._nas_share_type == NasShareType.NFS:
            req.suffix = '/NFS_SHARE_AUTH_CLIENT'
            self._build_share_req_body(share_id, user_name_or_ip, related_parameters, body_dict)
            req.body = json.dumps(body_dict)
        elif self._nas_share_type == NasShareType.CIFS:
            req.suffix = '/CIFS_SHARE_AUTH_CLIENT'
            self._build_share_req_body(share_id, user_name_or_ip, related_parameters, body_dict)
            req.body = json.dumps(body_dict)
        else:
            error_code, error_desc = ReturnCode.FAILED, f'The protocol {self._nas_share_type.name} is not supported.'
            log.error(f"Add share client fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        log.debug(f"Start to add share client, user_name_or_ip:{user_name_or_ip}")
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == StorageErrorCode.ALREADY_IN_WHITE:
            log.debug(f"Share client already exist, share_id:{share_id}, user_name_or_ip:{user_name_or_ip}")
            return ReturnCode.SUCCESS, ''
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Add share client fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def query_share_client(self, share_id, share_client_info_list: list):
        """
        :param share_id: Generated by create_share(), the value can be obtained through query_share_info().
        :param share_client_info_list:
        :return:
        """
        share_client_info_list.clear()
        req = HttpRequest()
        req.method = 'GET'
        if self._nas_share_type == NasShareType.NFS:
            req.suffix = f'/NFS_SHARE_AUTH_CLIENT'
            tmp_req_body = {
                'PARENTID': f'{share_id}',
            }
            req.body = json.dumps(tmp_req_body)
        elif self._nas_share_type == NasShareType.CIFS:
            req.suffix = '/CIFS_SHARE_AUTH_CLIENT'
            tmp_req_body = {
                'PARENTID': f'{share_id}',
            }
        else:
            error_code, error_desc = ReturnCode.FAILED, f'The protocol {self._nas_share_type.name} is not supported.'
            log.error(f"Query share client fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        req.body = json.dumps(tmp_req_body)
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            data = rsp.get('data', None)
            if data is None:
                error_code, error_desc = ReturnCode.NOT_EXIST, 'The share client data is null.'
                log.debug(f"Query share client not exist, desc: {error_desc}")
                return error_code, error_desc
            for data in rsp.get('data'):
                shared_client_info = NasSharedClientInfo()
                shared_client_info.id = data.get('ID')
                shared_client_info.name = data.get('NAME')
                shared_client_info.parent_id = data.get('PARENTID')
                if self._nas_share_type == NasShareType.NFS:
                    shared_client_info.permission = data.get('ACCESSVAL')
                else:
                    shared_client_info.permission = data.get('PERMISSION')
                    shared_client_info.cifs_domain_type = data.get('DOMAINTYPE')
                share_client_info_list.append(shared_client_info)
            return ReturnCode.SUCCESS, ''
        log.error(f"Query share client fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def delete_share_client(self, share_id, user_name_or_ip):
        """
        :param share_id:
        :param user_name_or_ip: self._nas_share_type=cifs, if value=None, delete all cifs client,
                                else delete specify client
                                self._nas_share_type=nfs, if value=None, delete all nfs client,
                                else delete specify client
        :return:
        """
        # 查询指定共享share_id下的白名单信息
        share_client_info_list = []
        current_client_name_dict = {}
        error_code, error_desc = self.query_share_client(share_id, share_client_info_list)
        if error_code == ReturnCode.SUCCESS:
            # 提取白名单中存在的所有客户端名称，判断本次待删除的客户端是否在该白名单中
            for share_client_info in share_client_info_list:
                current_client_name_dict[share_client_info.name] = share_client_info.id
        # 当前白名单为空，直接返回成功
        if not current_client_name_dict:
            log.debug(f'Delete share client success, current_client_name_dict is null.')
            return ReturnCode.SUCCESS, ''
        # 指定的待删除客户端不在当前白名单中，直接返回成功
        if user_name_or_ip and not current_client_name_dict.get(user_name_or_ip):
            log.debug(f'Delete share client success, user_name_or_ip:{user_name_or_ip}, '
                      f'Not in current_client_name_dict:{current_client_name_dict}.')
            return ReturnCode.SUCCESS, ''
        # 指定的待删除客户端在当前白名单中，删除该客户端
        if user_name_or_ip and current_client_name_dict.get(user_name_or_ip):
            return self.delete_share_client_by_id(current_client_name_dict.get(user_name_or_ip))
        # 未指定待删除客户端, 则删除所有的客户端
        if not user_name_or_ip:
            log.debug(f'Not specify deleted user_name_or_ip, delete all user_name_or_ip.')
            for share_client_info in share_client_info_list:
                error_code, error_desc = self.delete_share_client_by_id(share_client_info.id)
                if error_code != ReturnCode.SUCCESS:
                    return error_code, error_desc
        return ReturnCode.SUCCESS, ''

    def delete_share_client_by_id(self, client_id):
        log.info(f"Start to delete share client by id, client_id:{client_id}")
        req = HttpRequest()
        req.method = 'DELETE'
        if self._nas_share_type == NasShareType.NFS:
            req.suffix = f'/NFS_SHARE_AUTH_CLIENT/{client_id}'
        elif self._nas_share_type == NasShareType.CIFS:
            req.suffix = f'/CIFS_SHARE_AUTH_CLIENT/{client_id}'
        else:
            error_code, error_desc = ReturnCode.FAILED, f'The protocol {self._nas_share_type.name} is not supported.'
            log.error(f"Delete share client fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == StorageErrorCode.SHARE_CLIENT_NOT_EXIST:
            log.debug(f"Delete share client not exist, client_id:{client_id}")
            return ReturnCode.SUCCESS, ''
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Delete share client by id fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def create_snapshot(self, file_system_name, snapshot_name):
        req = HttpRequest()
        req.method = 'POST'
        req.suffix = f'/FSSNAPSHOT'
        req.body = json.dumps({
            'NAME': snapshot_name,
            'PARENTNAME': file_system_name,
            'PARENTTYPE': FILE_SYSTEM_TYPE
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Create snapshot fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def query_snapshot_by_snapshot_name(self, file_system_name, snapshot_name, snapshot_info: NasSnapshotInfo,
                                        vstore_id=''):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f"/fssnapshot"
        body_dict = {
            "PARENTNAME": file_system_name,
            "NAME": snapshot_name,
            "is_sort": "false"
        }
        if vstore_id:
            body_dict["vstoreId"] = vstore_id
        req.body = json.dumps(body_dict)

        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Query snapshot fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        if (not rsp.get('data')) or (not isinstance(rsp.get('data'), list)):
            error_code, error_desc = ReturnCode.FAILED, f'The rsp data is null.'
            log.debug(f"Query snapshot not exist, desc: {error_desc}")
            return error_code, error_desc
        # 查询快照 filter 规则：是包含，而不是等于，如：存在快照 snap1、snap12、snap123，查询快照名称=snap1
        # 接口会返回 snap1、snap12、snap123
        for snapshot in rsp.get('data'):
            # OP和Pacific的快照名称均以Snapshot_ + 任务id的形式，其中任务id连接符改为下划线
            if snapshot_name.replace("-", "_") in snapshot.get('NAME') or snapshot_name in snapshot.get('NAME'):
                snapshot_info.snapshot_id = snapshot.get('ID')
                snapshot_info.snapshot_name = snapshot.get('NAME')
                snapshot_info.file_system_id = snapshot.get('PARENTID')
                snapshot_info.file_system_name = snapshot.get('PARENTNAME')
                snapshot_info.dtree_id = snapshot.get("dtreeId")
                snapshot_info.dtree_path = snapshot.get("dtreePath")
                return ReturnCode.SUCCESS, ''
        error_desc = f'The snapshot {snapshot_name} is not found.'
        log.debug(f"Query snapshot not exist, desc: {error_desc}")
        return ReturnCode.FAILED, error_desc

    def query_file_system_snapshot_list(self, file_system_name, snapshot_info_list: list):
        snapshot_info_list.clear()
        req = HttpRequest()
        req.method = 'GET'

        req.suffix = f'/fssnapshot'
        req.body = json.dumps({
            'PARENTNAME': file_system_name,
            'is_sort': "true",
            "sort_param": "TIMESTAMP,d"
        })

        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            if (not rsp.get('data')) or (not isinstance(rsp.get('data'), list)):
                error_code, error_desc = ReturnCode.FAILED, f'The rsp data is null.'
                log.debug(f"Query snapshot list not exist, code: {error_code}, desc: {error_desc}")
                return error_code, error_desc
            for snap in rsp.get('data'):
                snapshot_info = NasSnapshotInfo()
                snapshot_info.snapshot_id = snap.get('ID')
                snapshot_info.snapshot_name = snap.get('NAME')
                snapshot_info.file_system_id = snap.get('PARENTID')
                snapshot_info.file_system_name = snap.get('PARENTNAME')
                snapshot_info_list.append(snapshot_info)
            if not snapshot_info_list:
                error_desc = f'The filesystem snapshot is null.'
                log.debug(f"Query snapshot list not exist, desc: {error_desc}")
                return ReturnCode.FAILED, error_desc
            return ReturnCode.SUCCESS, ''
        else:
            log.error(f"Query snapshot list fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc

    def delete_snapshot(self, file_system_name, snapshot_name):
        snapshot_info = NasSnapshotInfo()
        error_code, error_desc = self.query_snapshot_by_snapshot_name(file_system_name, snapshot_name, snapshot_info)
        if error_code == ReturnCode.FAILED:
            return ReturnCode.SUCCESS, ''
        req = HttpRequest()
        req.method = 'DELETE'
        req.suffix = f'/FSSNAPSHOT/{snapshot_info.snapshot_id}'
        req.body = json.dumps({
            "name": snapshot_name,
            "namespace_id": snapshot_info.file_system_id,
            "dtree_id": snapshot_info.dtree_id
        })

        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Delete snapshot fail, error code: {error_code}, error desc: {error_desc}")
        else:
            log.debug(f"Delete snapshot {snapshot_name} success.")
        return error_code, error_desc

    def create_clone_file_system_from_snapshot(self, clone_file_system_name, source_file_system_name,
                                               snapshot_name, character_set=0, vstore_id=''):
        device_details = DeviceDetails()
        is_worm_file_system_create_needed = True
        error_code, error_desc = self.query_file_system(clone_file_system_name, device_details)
        if error_code == ReturnCode.SUCCESS and device_details.parent_file_system_id:
            error_desc = f'Create clone file system failed, file system name {clone_file_system_name} already exist!'
            log.error(f"Error_desc:{error_desc}")
            return ReturnCode.FAILED, error_desc
        elif error_code == ReturnCode.SUCCESS and not device_details.parent_file_system_id:
            is_worm_file_system_create_needed = False
        snapshot_info = NasSnapshotInfo()
        error_code, error_desc = self.query_snapshot_by_snapshot_name(source_file_system_name, snapshot_name,
                                                                      snapshot_info, vstore_id)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc

        device_details_worm = DeviceDetails()
        error_code, error_desc = self.query_file_system(source_file_system_name, device_details_worm)
        if error_code != ReturnCode.SUCCESS:
            error_desc = f'Failed to find source fs which needs to be cloned, file system name:' \
                         f'{clone_file_system_name}, source file system name: {source_file_system_name}'
            log.error(f"Error_desc:{error_desc}")
            return ReturnCode.FAILED, error_desc
        # 当需要克隆的文件系统是worm时，worm_type值为1， 普通文件系统worm_type为0
        is_worm_on = bool(device_details_worm.worm_type)
        if is_worm_on and is_worm_file_system_create_needed:
            error_code, error_desc = self.create_file_system(file_system_name=clone_file_system_name,
                                                             source_fs_name=source_file_system_name,
                                                             character_set=character_set)
            if error_code != ReturnCode.SUCCESS:
                error_desc = f'Failed to create a normal file system: {clone_file_system_name} for a worm file system' \
                             f' to be cloned on'
                log.error(f"Error_desc:{error_desc}")
                return ReturnCode.FAILED, error_desc
            log.info(f'Succeed to create a normal file system: {clone_file_system_name} for worm to be cloned')

        req = HttpRequest()
        req.method = 'POST'
        req.suffix = '/filesystem'
        dump_body = {
            'NAME': clone_file_system_name,
            'ALLOCTYPE': f'{SpaceAllocationType.THIN_FILE_SYSTEM}',
            'PARENTSNAPSHOTNAME': snapshot_name,
            'PARENTFILESYSTEMID': snapshot_info.file_system_id
        }
        if is_worm_on:
            dump_body['useExistFs'] = True
        req.body = json.dumps(dump_body)
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f'Create clone file system failed, error code: {error_code}, error desc: {error_desc}')
            return error_code, error_desc
        return self.wait_file_system_status_ready(clone_file_system_name, device_details)

    def wait_file_system_status_ready(self, file_system_name, device_details: DeviceDetails):
        settings = get_settings()
        for _ in range(0, settings.STATUS_QUERY_TIMEOUT_SEC):
            error_code, error_desc = self.query_file_system(file_system_name, device_details)
            if error_code != ReturnCode.SUCCESS:
                return error_code, error_desc
            if int(device_details.running_status) == RunningStatusEnum.ONLINE \
                    and int(device_details.read_write_status) == ReadWriteStatusEnum.READ_WRITE:
                return ReturnCode.SUCCESS, ''
            else:
                time.sleep(settings.STATUS_QUERY_INTERVAL_SEC)
        error_code, error_desc = ReturnCode.FAILED, 'Wait file system status ready timeout.'
        log.error(f"Wait file system status ready fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def revert_file_system_by_snapshot(self, file_system_name, snapshot_name):
        req = HttpRequest()
        req.method = 'PUT'
        req.suffix = '/FSSNAPSHOT/ROLLBACK_FSSNAPSHOT'
        req.body = json.dumps({
            'rollbackTargetObjName': snapshot_name,
            'PARENTNAME': file_system_name,
            'rollbackSpeed': ROLL_BACK_SPEED
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            return self.wait_rollback_snapshot_complete(file_system_name)
        else:
            log.error(f"Revert file system by snapshot fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc

    def wait_rollback_snapshot_complete(self, file_system_name):
        revert_info = RevertInfo()
        settings = get_settings()
        for _ in range(settings.STATUS_QUERY_TIMEOUT_SEC):
            error_code, error_desc = self.query_file_system_revert_info(file_system_name, revert_info)
            if error_code != ReturnCode.SUCCESS:
                return error_code, error_desc
            finished_by_timestamp = all([self.is_available_timestamp(revert_info.rollback_start_time),
                                         self.is_available_timestamp(revert_info.rollback_end_time),
                                         (revert_info.rollback_end_time > revert_info.rollback_start_time)])
            if revert_info.rollback_status == RollbackStatusEnum.ROLLBACK_STATUS_IDLE and \
                    (revert_info.rollback_rate >= MAX_ROLLBACK_RATE or finished_by_timestamp):
                return ReturnCode.SUCCESS, ''
            time.sleep(settings.STATUS_QUERY_INTERVAL_SEC)
        error_desc = 'Revert file system by snapshot timeout.'
        log.error(f"Revert file system by snapshot fail, error desc: {error_desc}")
        return ReturnCode.FAILED, error_desc

    def query_file_system_revert_info(self, file_system_name, revert_info: RevertInfo):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/FSSNAPSHOT/QUERY_FS_SNAPSHOT_ROLLBACK?PARENTNAME={file_system_name}'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            if not rsp.get('data'):
                error_desc = f'Query file system revert info not exist, data is null.'
                log.debug(f"{error_desc}")
                return ReturnCode.FAILED, error_desc
            revert_info.rollback_rate = int(rsp.get('data').get('rollbackRate', 0))
            revert_info.rollback_status = int(rsp.get('data').get('rollbackStatus', 0))
            revert_info.rollback_speed = int(rsp.get('data').get('rollbackSpeed', 0))
            revert_info.rollback_start_time = int(rsp.get('data').get('rollbackStarttime', 0))
            revert_info.rollback_end_time = int(rsp.get('data').get('rollbackEndtime', 0))
            return ReturnCode.SUCCESS, ''
        else:
            log.error(f"Query file system revert info fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc

    def create_dtree(self, file_system_name, dtree_name, security_style=DtreeSecurityStyleEnum.UNIX):
        dtree_info = DtreeInfo()
        error_code, error_desc = self.query_dtree(file_system_name, dtree_name, dtree_info)
        if error_code == ReturnCode.SUCCESS:
            return ReturnCode.SUCCESS, ''
        req = HttpRequest()
        req.method = 'POST'
        req.suffix = '/QUOTATREE'
        req.body = json.dumps({
            'NAME': dtree_name,
            'PARENTTYPE': f'{DtreeParentTypeEnum.FILESYSTEM}',
            'PARENTNAME': file_system_name,
            'QUOTASWITCH': 'false',
            'securityStyle': f'{security_style}'
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        # quota tree数量已达规格限制
        if error_code in {StorageErrorCode.QTREE_REACH_MAX, StorageErrorCode.DTREE_REACH_MAX}:
            log.warn(f"Create dtree upper limit, error code: {error_code}, error desc: {error_desc}")
            raise CustomException({"code": error_code, "message": error_desc})
        if error_code == StorageErrorCode.DTREE_NAME_IS_EXIST:
            return ReturnCode.SUCCESS, ''
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Create dtree fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def query_dtree(self, file_system_name, dtree_name, dtree_info: DtreeInfo):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/QUOTATREE?PARENTNAME={file_system_name}&NAME={dtree_name}'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            if not rsp.get('data'):
                error_desc = f'Query dtree info not exist, data is null.'
                log.debug(f"{error_desc}")
                return ReturnCode.NOT_EXIST, error_desc
            dtree_info.dtree_name = rsp.get('data').get('NAME')
            dtree_info.dtree_id = rsp.get('data').get('ID')
            dtree_info.file_system_id = rsp.get('data').get('PARENTID')
            dtree_info.path = rsp.get('data').get('path')
            dtree_info.quota_switch = rsp.get('data').get('QUOTASWITCH')
            dtree_info.quota_switch_status = rsp.get('data').get('QUOTASWITCHSTATUS')
            dtree_info.security_style = rsp.get('data').get('securityStyle')
            dtree_info.file_system_name = file_system_name
            return ReturnCode.SUCCESS, ''
        else:
            if error_code == StorageErrorCode.DTREE_NOT_EXIST:
                log.debug(f'Query dtree {dtree_name}: {error_desc}')
            else:
                log.error(f"Query dtree fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc

    def delete_dtree(self, file_system_name, dtree_name):
        dtree_info = DtreeInfo()
        error_code, error_desc = self.query_dtree(file_system_name, dtree_name, dtree_info)
        if error_code in {ReturnCode.NOT_EXIST, StorageErrorCode.DTREE_NOT_EXIST,
                          StorageErrorCode.RETURN_DTREE_FS_NOT_EXIST}:
            return ReturnCode.SUCCESS, ''
        req = HttpRequest()
        req.method = 'DELETE'
        req.suffix = f'/QUOTATREE?PARENTNAME={file_system_name}&NAME={dtree_name}'
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code in {StorageErrorCode.DTREE_NOT_EXIST, StorageErrorCode.RETURN_DTREE_FS_NOT_EXIST}:
            return ReturnCode.SUCCESS, ''
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Delete dtree fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def bind(self, file_system_name, user_name_or_ip, share_name=None, is_ip_sec_open=False,
             file_handle_byte_alignment=False):
        device_details = DeviceDetails()
        error_code, error_desc = self.query_file_system(file_system_name, device_details)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        error_code, error_desc = self.create_share(file_system_name, device_details.device_id, share_name,
                                                   is_ip_sec_open, file_handle_byte_alignment)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        shared_info = NasShareInfo()
        error_code, error_desc = self.query_share_info(device_details.device_id, shared_info, share_name)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        related_parameters = {'ACCESSKRB5P': f'{KERBEROS_5P_OPEN}'} if is_ip_sec_open else None
        return self.add_share_client(shared_info.share_id, user_name_or_ip, related_parameters)

    def bind_cifs_dtree(self, file_system_name, user_name_or_ip, share_name=None, is_ip_sec_open=False):
        device_details = DeviceDetails()
        error_code, error_desc = self.query_file_system(file_system_name, device_details)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        shared_info = NasShareInfo()
        error_code, error_desc = self.query_share_info(device_details.device_id, shared_info, share_name)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        related_parameters = {'ACCESSKRB5P': f'{KERBEROS_5P_OPEN}'} if is_ip_sec_open else None
        return self.add_share_client(shared_info.share_id, user_name_or_ip, related_parameters)

    def unbind(self, file_system_name, user_name_or_ip, share_name=None):
        device_details = DeviceDetails()
        error_code, error_desc = self.query_file_system(file_system_name, device_details)
        if error_code == ReturnCode.NOT_EXIST:
            log.warn(f'Unbind, filesystem not exist, name:{file_system_name}')
            return ReturnCode.SUCCESS, ''
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        shared_info = NasShareInfo()
        error_code, error_desc = self.query_share_info(device_details.device_id, shared_info, share_name)
        if error_code == ReturnCode.NOT_EXIST:
            return ReturnCode.SUCCESS, ''
        elif error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        return self.delete_share_client(shared_info.share_id, user_name_or_ip)

    def bind_for_dtree(self, file_system_name, dtree_path, client_list: list, is_ip_sec_open=False):
        device_details = DeviceDetails()
        error_code, error_desc = self.query_file_system(file_system_name, device_details)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        shared_info = NasShareInfo()
        error_code, error_desc = self.query_share_info_by_dtree_path(device_details.device_id, dtree_path, shared_info)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        related_parameters = {'ACCESSKRB5P': f'{KERBEROS_5P_OPEN}'} if is_ip_sec_open else None
        for ip in client_list:
            res, desc = self.add_share_client(shared_info.share_id, ip, related_parameters)
            if res != ReturnCode.SUCCESS:
                return res, desc
        return ReturnCode.SUCCESS, ''

    def unbind_for_dtree(self, file_system_name, dtree_path, user_name_or_ip):
        device_details = DeviceDetails()
        error_code, error_desc = self.query_file_system(file_system_name, device_details)
        if error_code == ReturnCode.NOT_EXIST:
            log.warn(f'Unbind, filesystem not exist, name:{file_system_name}')
            return ReturnCode.SUCCESS, ''
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        shared_info = NasShareInfo()
        error_code, error_desc = self.query_share_info_by_dtree_path(device_details.device_id, dtree_path, shared_info)
        if error_code == ReturnCode.NOT_EXIST:
            return ReturnCode.SUCCESS, ''
        elif error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        return self.delete_share_client(shared_info.share_id, user_name_or_ip)

    def create_smart_qos(self, file_system_name, qos_set_info: SmartQoSInfo):
        if not file_system_name or not qos_set_info.name:
            error_desc = f'file system name:{file_system_name} or qos name:{qos_set_info.name} is null.'
            return ReturnCode.FAILED, error_desc
        if len(qos_set_info.name) > QOS_NAME_MAX_LENGTH:
            qos_set_info.name = qos_set_info.name[:QOS_NAME_MAX_LENGTH]
        qos_info_tmp = SmartQoSInfo()
        try:
            error_code, error_desc = self.query_smart_qos(qos_set_info.name, qos_info_tmp)
        except CustomException as e:
            return e.error_code, e.get_error_rsp()
        if error_code == ReturnCode.FAILED:
            # Qos不存在，创建Qos
            device_details = DeviceDetails()
            error_code, error_desc = self.query_file_system(file_system_name, device_details)
            if error_code != ReturnCode.SUCCESS:
                log.error(f"Create qos fail, file system error: {error_code}, error desc: {error_desc}")
                return error_code, error_desc
            if device_details.io_class_id:
                log.warn(f"Qos exist id:{device_details.io_class_id}, remove from fs id:{device_details.device_id}")
                error_code, error_desc = self.remove_exist_qos_associate(device_details.io_class_id)
                if error_code != ReturnCode.SUCCESS:
                    return error_code, error_desc
            qos_set_info.fs_id_list = [f'{device_details.device_id}']
            error_code, error_desc = self._create_qos(qos_set_info)
            if error_code != ReturnCode.SUCCESS:
                return error_code, error_desc
            return self.active_smart_qos(qos_set_info.id, True)
        elif error_code == ReturnCode.SUCCESS:
            # Qos存在，处理Qos
            error_code, error_desc = self.handle_smart_qos(file_system_name, qos_set_info, qos_info_tmp)
        return error_code, error_desc

    def handle_smart_qos(self, file_system_name, qos_set_info, qos_info_tmp):
        # 更新Qos
        error_code, error_desc = self.update_smart_qos(qos_info_tmp.id, qos_set_info)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Update qos fail, file system error: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        device_details = DeviceDetails()
        error_code, error_desc = self.query_file_system(file_system_name, device_details)
        if device_details.io_class_id and device_details.io_class_id == qos_info_tmp.id:
            return error_code, error_desc
        if device_details.io_class_id and device_details.io_class_id != qos_info_tmp.id:
            # 文件系统上当前qos和目标qos不同，重新绑定至目标qos
            log.warn(f"File system exist qos id:{device_details.io_class_id}, dest qos id:{qos_info_tmp.id}")
            error_code, error_desc = self.remove_exist_qos_associate(device_details.io_class_id)
            if error_code != ReturnCode.SUCCESS:
                log.error(f"Update qos fail, file system error: {error_code}, error desc: {error_desc}")
                return error_code, error_desc
        if not qos_info_tmp.fs_id_list or device_details.device_id not in qos_info_tmp.fs_id_list:
            qos_info_tmp.fs_id_list.append(f'{device_details.device_id}')
        error_code, error_desc = self.set_smart_qos_associate(qos_info_tmp.id, qos_info_tmp.fs_id_list)
        return error_code, error_desc

    def remove_exist_qos_associate(self, exist_qos_id):
        qos_info_exist_tmp = SmartQoSInfo()
        try:
            error_code, error_desc = self.query_smart_qos_by_id(exist_qos_id, qos_info_exist_tmp)
        except CustomException as error_info:
            return error_info.error_code, error_info.get_error_rsp()
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        return self.set_smart_qos_associate(exist_qos_id, [])

    def update_dt_user_password(self, username, current_password, new_password):
        req = HttpRequest()
        req.method = 'PUT'
        req.suffix = '/initialize_user_pwd'
        req.body = json.dumps({
            "PASSWORD": f'{new_password}',
            'ID': f'{username}',
            'IMPORTANTPSW': f'{current_password}',
            'vstoreId': f'{DATA_TURBO_VSTOREID}'
        })

        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            log.info(f'Update data turbo user:{username} password success.')
        else:
            log.error(f'Update data turbo user password fail, error code: {error_code}, error desc: {error_desc}')
        return error_code, error_desc

    def create_data_turbo_user(self, username, password):
        req = HttpRequest()
        req.method = 'POST'
        req.suffix = '/user'
        req.body = json.dumps({
            'NAME': f'{username}',
            'PASSWORD': f'{password}',
            'ROLEID': f'{DATA_TURBO_ROLE_ID}',
            'SCOPE': DATA_TURBO_SCOPE,
            'vstoreId': f'{DATA_TURBO_VSTOREID}'
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            log.info(f'Create data turbo user:{username} success.')
        else:
            log.error(f'Create data turbo user fail, error code: {error_code}, error desc: {error_desc}')
        return error_code, error_desc

    def query_data_turbo_user_info(self, user_name):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/user/{user_name}'
        req.body = json.dumps({
            'vstoreId': f'{DATA_TURBO_VSTOREID}'
        })
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            log.info(f'Data turbo user:{user_name} exist.')
        elif error_code == StorageErrorCode.DATA_TURBO_USER_NOT_EXIST:
            log.info(f'Data turbo user:{user_name} does not exist, error code: {error_code}, error desc: {error_desc}')
        elif error_code == StorageErrorCode.DATA_TURBO_USER_OFFLINE:
            log.info(f"Found user:{user_name} but is offline, error code: {error_code}, error desc: {error_desc}")
        else:
            log.error(f"Query data turbo user fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def create_windows_user(self, user_name, password):
        windows_user_info = WindowsUserInfo()
        error_code, error_desc = self.query_windows_user_info(user_name, windows_user_info)
        if error_code == ReturnCode.SUCCESS:
            return error_code, error_desc
        req = HttpRequest()
        req.method = 'POST'
        req.suffix = '/WINDOWS_USER'
        req.body = json.dumps({
            'NAME': f'{user_name}',
            'PASSWORD': f'{password}'
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            log.info(f'Create windows user:{user_name} success.')
        else:
            log.error(f"Create windows user fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def update_windows_user(self, user_name, password):
        req = HttpRequest()
        req.method = 'PUT'
        req.suffix = f'/WINDOWS_USER'
        req.body = json.dumps({
            'NAME': f'{user_name}',
            'PASSWORD': f'{password}'
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code in {ReturnCode.SUCCESS, StorageErrorCode.WINDOWS_USER_PASSWORD_SAME}:
            log.info(f'Update windows user:{user_name} success.')
            error_code = ReturnCode.SUCCESS
        else:
            log.error(f"Update windows user fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def query_windows_user_info(self, user_name, windows_user_info: WindowsUserInfo):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/WINDOWS_USER?NAME={user_name}'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            if not rsp.get('data'):
                error_desc = f'Query windows user info not exist, data is null.'
                log.debug(f"{error_desc}")
                return ReturnCode.FAILED, error_desc
            windows_user_info.name = rsp.get('data').get('NAME')
            windows_user_info.rid = rsp.get('data').get('RID')
            windows_user_info.password_validity_period = rsp.get('data').get('PASSWORDVALIDPERIOD')
            windows_user_info.status = True if rsp.get('data').get('STATUSENABLE') == "true" else False
            windows_user_info.privileges = rsp.get('data').get('PRIVILEGES')
            windows_user_info.group_name_list = rsp.get('data').get('GROUPNAMES')
            return ReturnCode.SUCCESS, ''
        else:
            if error_code == StorageErrorCode.WINDOWS_USER_NOT_EXIST:
                log.debug(f'Query windows user {user_name}: {error_desc}')
            else:
                log.error(f"Query windows user fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc

    def delete_data_turbo_user(self, username):
        req = HttpRequest()
        req.method = 'DELETE'
        req.suffix = f'/user/{username}'
        req.body = json.dumps({
            'vstoreId': f'{DATA_TURBO_VSTOREID}'
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            log.info(f'Delete data turbo user:{username} success.')
        elif error_code == StorageErrorCode.DATA_TURBO_USER_NOT_EXIST:
            log.info(f'User:{username} does not exist, error code: {error_code}, error desc: {error_desc}')
        else:
            log.error(f'Delete data turbo user fail, error code: {error_code}, error desc: {error_desc}')
        return error_code, error_desc

    def delete_windows_user(self, user_name):
        windows_user_info = WindowsUserInfo()
        error_code, error_desc = self.query_windows_user_info(user_name, windows_user_info)
        if error_code == ReturnCode.FAILED or error_code == StorageErrorCode.WINDOWS_USER_NOT_EXIST:
            return ReturnCode.SUCCESS, ''
        req = HttpRequest()
        req.method = 'DELETE'
        req.suffix = f'/WINDOWS_USER?NAME={user_name}'
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            log.info(f'Delete windows user:{user_name} success.')
        else:
            log.error(f"Delete windows user fail, error code: {error_code}, error desc: {error_desc}")
        return error_code, error_desc

    def is_keep_old_file_system(self, file_system_name: str, storage_info: dict):
        device_details = DeviceDetails()
        # 新文件系统命名方式会以 su{存储池id} 作为唯一标识，如果检测到有文件系统为老版本命名方式，则需要保留
        temp_name_list = file_system_name.split("_")
        old_file_system_name = ""
        for i in range(len(temp_name_list)):
            if i != 1 and temp_name_list[i].startswith("su") and temp_name_list[i][2].isdigit():
                continue
            old_file_system_name += f"{temp_name_list[i]}_"
        old_file_system_name = old_file_system_name[:-1]

        error_code, error_desc = self.query_file_system(old_file_system_name, device_details)
        # 查询结果需要满足两个条件才需要保留老版本文件系统名：1、文件名和老命名格式一致；2、存储池号一样
        if error_code != ReturnCode.SUCCESS:
            log.info(f"There does NOT have any filesystem named according to previous versions' rule. "
                     f"Do NOT need to keep old file system name. Need to create new filesystem.")
            return False, ""
        if str(device_details.parent_id) != str(storage_info.get("storage_pool", "")):
            log.info(f"The filesystem waiting to be created does NOT existed in the target storage pool. "
                     f"Do NOT need to keep old file system name. Need to create new filesystem name.")
            return False, ""
        # 保留老文件系统名
        log.info(f"The file system waiting to be created has already been existed. "
                 f"Keep the old file system name: {old_file_system_name}.")
        return True, old_file_system_name

    def insert_windows_user_to_group(self, user_name, group_name, account_type):
        req = HttpRequest()
        req.method = 'POST'
        req.suffix = '/WINDOWS_USER_GROUP_MAP'
        req.body = json.dumps({
            'NAME': f'{user_name}',
            'GROUPNAME': f'{group_name}',
            'ACCOUNTTYPE': f'{account_type}'
        })
        rsp = self._send_request(req, lock_session=True)
        code, desc = self._check_response(rsp)
        if code != ReturnCode.SUCCESS:
            log.error(f'Failed to insert windows user to group, user: {user_name}, group: {group_name}.')
            raise Exception(RequestResult(ErrorCode=code, ErrorDesc=desc).dict())

    def list_windows_user_in_group(self, group_name, account_type):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = '/WINDOWS_USER_GROUP_MAP'
        req.body = json.dumps({
            'GROUPNAME': f'{group_name}',
            'ACCOUNTTYPE': f'{account_type}'
        })
        rsp = self._send_request(req, lock_session=True)
        code, desc = self._check_response(rsp)
        if code != ReturnCode.SUCCESS:
            log.error(f'Failed to get window user, group: {group_name}.')
            raise Exception(RequestResult(ErrorCode=code, ErrorDesc=desc).dict())
        list_user = list()
        if rsp.get('data'):
            list_user.extend(ResUserInGroup.parse_obj(item) for item in rsp.get('data'))
        return list_user

    def get_real_time_performance_info(self, fs_id_set, performance_data: list):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = f'/performance_data?object_type={FILE_SYSTEM_TYPE}&indicators={FILE_SYSTEM_BANDWIDTH}' \
                     f'&object_list={",".join(str(i) for i in fs_id_set)}'
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Get performance data info fail, error code:{error_code}, error desc:{error_desc}")
            return error_code, error_desc
        data = rsp.get('data', None)
        if not data:
            error_desc = 'Query performance info not exist, desc: The data is null.'
            log.error(f"Error_desc:{error_desc}")
            return ReturnCode.FAILED, error_desc
        performance_data.extend(data)
        return ReturnCode.SUCCESS, ''

    def get_avg_performance_info(self, begin_time, end_time, ids: list, performance_data: list):
        req = HttpRequest()
        req.method = 'POST'
        req.body = json.dumps({
            "begin_time": begin_time,
            "end_time": end_time,
            "objects": [
                {
                    "ids": ids,
                    "indicators": [FILE_SYSTEM_BANDWIDTH],
                    "object_type": FILE_SYSTEM_TYPE
                }
            ],
            "compute_mode": "avg"
        })
        req.suffix = '/performance_data'
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        data = rsp.get('data', None)
        if not data:
            error_desc = 'Query performance info not exist, desc: The data is null.'
            log.error(f"Error_desc:{error_desc}")
            return ReturnCode.FAILED, error_desc
        performance_data.extend(data)
        return error_code, error_desc

    def create_vpc_mapping(self, mark_id, vpc_id, vstore_id=DEFAULT_VSTOREID):
        req = HttpRequest()
        req.method = 'POST'
        req.suffix = '/nfs_vpc_mapping'
        req.body = json.dumps({
            'markId': mark_id,
            'vpcId': vpc_id,
            'vstoreId': vstore_id
        })
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            log.info(f'Create vpc mapping success')
        else:
            log.error(f'Create vpc mapping failed, error_code: {error_code}, error_desc: {error_desc}.')
        return error_code, error_desc

    def delete_vpc_mapping(self, mark_id):
        req = HttpRequest()
        req.method = 'DELETE'
        req.suffix = f'/nfs_vpc_mapping/{mark_id}'
        rsp = self._send_request(req)
        error_code, error_desc = self._check_response(rsp)
        if error_code == ReturnCode.SUCCESS:
            log.info(f'Delete vpc mapping success')
        else:
            log.error(f'Delete vpc mapping failed, error_code: {error_code}, error_desc: {error_desc}.')
        return error_code, error_desc

    def get_cifs_ip_controller_rule(self, share_id, controller_rules: list):
        req = HttpRequest()
        req.method = 'GET'
        req.suffix = '/CIFS_IP_CONTROL_RULE'
        rsp = self._send_request(req, lock_session=True)
        code, desc = self._check_response(rsp)
        if code != ReturnCode.SUCCESS:
            log.error(f'Failed to get cifs ip controller rule, share_id: {share_id}.')
            return code, desc
        data = rsp.get('data', None)
        if not data:
            error_desc = 'Query cifs ip controller info not exist, desc: The data is null.'
            log.error(f"Error_desc:{error_desc}")
            return ReturnCode.FAILED, error_desc
        controller_rule = CifsIpControllerRule()
        for rule in data:
            if rule.get('cifsShareID') == share_id:
                controller_rule.vstore_id = rule.get('vstoreId')
                controller_rule.id = rule.get('ID')
                controller_rule.cifs_share_id = rule.get('cifsShareID')
                controller_rule.rule = rule.get('rule')
                controller_rules.append(controller_rule)
        return code, desc

    def add_cifs_ip_controller_rule(self, device_details, ip, share_name=None):
        shared_info = NasShareInfo()
        error_code, error_desc = self.query_share_info(device_details.device_id, shared_info, share_name)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        controller_rules = []
        error_code, error_desc = self.get_cifs_ip_controller_rule(shared_info.share_id, controller_rules)
        if error_code == ReturnCode.SUCCESS:
            for rule in controller_rules:
                if rule.rule == ip:
                    log.info(f'{ip} exists in cifs ip controller rule, no need to add again, share_name:{share_name}')
                    return error_code, error_desc
        req = HttpRequest()
        req.method = 'POST'
        req.suffix = '/CIFS_IP_CONTROL_RULE'
        req.body = json.dumps({
            'cifsShareID': f'{shared_info.share_id}',
            'vstoreId': 0,
            'rule': ip
        })
        rsp = self._send_request(req, lock_session=True)
        code, desc = self._check_response(rsp)
        if code != ReturnCode.SUCCESS:
            log.error(f'Failed to add cifs ip controller rule, share_name: {share_name}, ip: {ip}.')
            return error_code, error_desc
        return error_code, error_desc

    def delete_cifs_ip_controller_rule(self, device_details, ip, share_name=None, dtree_path=None):
        shared_info = NasShareInfo()
        if dtree_path:
            error_code, error_desc = self.query_share_info_by_dtree_path(device_details.device_id,
                                                                         dtree_path, shared_info, share_name)
        else:
            error_code, error_desc = self.query_share_info(device_details.device_id, shared_info, share_name)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        controller_rules = []
        error_code, error_desc = self.get_cifs_ip_controller_rule(shared_info.share_id, controller_rules)
        if error_code != ReturnCode.SUCCESS:
            log.error(f'Failed to get cifs ip controller rule, share_name: {share_name}, ip: {ip}.')
            return error_code, error_desc
        rule_id = ''
        for rule in controller_rules:
            if rule.rule == ip:
                rule_id = rule.id
                break
        if not rule_id:
            return ReturnCode.SUCCESS, ""
        req = HttpRequest()
        req.method = 'DELETE'
        req.suffix = f'/CIFS_IP_CONTROL_RULE?ID={rule_id}'
        rsp = self._send_request(req, lock_session=True)
        code, desc = self._check_response(rsp)
        if code != ReturnCode.SUCCESS:
            log.error(f'Failed to add cifs ip controller rule, share_name: {share_name}, ip: {ip}.')
            return error_code, error_desc
        return error_code, error_desc

    def osa_create_clone_file_system_from_snapshot(self, osa_livemount: OsaLivemountInfo):
        clone_file_system_name = osa_livemount.clone_file_system_name
        source_file_system_name = osa_livemount.source_file_system_name
        snapshot_name = osa_livemount.snapshot_name
        device_details = DeviceDetails()
        error_code, error_desc = self.query_file_system(clone_file_system_name, device_details)
        if error_code == ReturnCode.SUCCESS:
            error_desc = f'Create clone file system failed, file system name {clone_file_system_name} already exist!'
            log.error(f"{error_desc}")
            return ReturnCode.FAILED, error_desc
        snapshot_info = NasSnapshotInfo()
        error_code, error_desc = self.query_snapshot_by_snapshot_name(source_file_system_name, snapshot_name,
                                                                      snapshot_info, osa_livemount.vstore_id)
        if error_code != ReturnCode.SUCCESS:
            return error_code, error_desc
        device_details_worm = DeviceDetails()
        error_code, error_desc = self.query_file_system(source_file_system_name, device_details_worm)
        if error_code != ReturnCode.SUCCESS:
            error_desc = f'Failed to find source fs which needs to be cloned, file system name:' \
                         f'{clone_file_system_name}, source file system name: {source_file_system_name}'
            log.error(f"Error_desc:{error_desc}")
            return ReturnCode.FAILED, error_desc
        # 当需要克隆的文件系统是worm时，worm_type值为1， 普通文件系统worm_type为0
        is_worm_on = bool(device_details_worm.worm_type)
        if is_worm_on:
            error_code, error_desc = self.create_file_system(file_system_name=clone_file_system_name,
                                                             source_fs_name=source_file_system_name,
                                                             character_set=osa_livemount.character_set)
            if error_code != ReturnCode.SUCCESS:
                error_desc = f'Failed to create a normal file system: {clone_file_system_name} for a worm file system' \
                             f' to be cloned on'
                log.error(f"Error_desc:{error_desc}")
                return ReturnCode.FAILED, error_desc
            log.info(f'Succeed to create a normal file system: {clone_file_system_name} for worm to be cloned')
        req = HttpRequest()
        req.method = 'POST'
        req.suffix = '/livemount'
        req.body = json.dumps({
            'name': clone_file_system_name, 'alloc_type': f'{SpaceAllocationType.THIN_FILE_SYSTEM}',
            'parent_snapshot_name': snapshot_info.snapshot_name, 'parent_filesystem_id': snapshot_info.file_system_id,
            'task_id': osa_livemount.task_id, 'repository': osa_livemount.repository,
            'device_name': device_details_worm.device_name, 'device_id': device_details_worm.device_id,
            'parent_snapshot_id': snapshot_info.snapshot_id
        })
        rsp = self._send_request(req, lock_session=True)
        error_code, error_desc = self._check_response(rsp)
        if error_code != ReturnCode.SUCCESS:
            log.error(f'Create clone file system failed, error code: {error_code}, error desc: {error_desc}')
            return error_code, rsp
        return error_code, rsp

    def create_converged_qos(self, file_system_name, qos_set_info: SmartQoSInfo):
        if not file_system_name or not qos_set_info.name:
            error_desc = f'file system name:{file_system_name} or qos name:{qos_set_info.name} is null.'
            return ReturnCode.FAILED, error_desc
        if len(qos_set_info.name) > CONVERGED_QOS_NAME_MAX_LENGTH:
            qos_set_info.name = qos_set_info.name[:CONVERGED_QOS_NAME_MAX_LENGTH]
        qos_info_same_name = SmartQoSInfo()
        try:
            err_code, err_desc = self.query_converged_qos_by_name(qos_set_info.name, qos_info_same_name)
        except CustomException as e:
            log.error(f"Query converged qos by name failed, code:{e.error_code}, error: {e.get_error_rsp()}")
            return e.error_code, e.get_error_rsp()
        if err_code == ReturnCode.SUCCESS:
            # Qos存在，处理Qos
            error_code, error_desc = self.handle_converged_qos(file_system_name, qos_set_info, qos_info_same_name)
            return error_code, error_desc
        elif err_code == ReturnCode.FAILED:
            # Qos不存在，创建Qos
            error_code, error_desc = self.query_converged_qos_association_by_fsname(file_system_name, qos_set_info)
            if error_code == ReturnCode.SUCCESS and qos_set_info.id:
                # 文件系统已经关联了其他qos策略，先解除关联
                log.warn(f"Qos exist id:{qos_set_info.id}, remove from fs name:{file_system_name}")
                error_code, error_desc = self.delete_converged_qos_association(file_system_name, qos_set_info)
                if error_code != ReturnCode.SUCCESS:
                    return error_code, error_desc
            error_code, error_desc = self._create_converged_qos(qos_set_info)
            # 创建的qos关联文件系统
            if error_code == ReturnCode.FAILED:
                log.error(f"Create converged qos fail, error: {error_code}, error desc: {error_desc}")
                return error_code, error_desc
            error_code, error_desc = self.add_converged_qos_association(file_system_name, qos_set_info)
        return error_code, error_desc

    def handle_converged_qos(self, file_system_name, qos_set_info, qos_info_same_name):
        # 更新Qos
        error_code, error_desc = self.update_converged_qos(qos_info_same_name.id, qos_set_info)
        if error_code != ReturnCode.SUCCESS:
            log.error(f"Update qos fail, file system error: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
        qos_info_filesystem_associated = SmartQoSInfo()
        error_code, error_desc = self.query_converged_qos_association_by_fsname(file_system_name,
                                                                                qos_info_filesystem_associated)
        if qos_info_filesystem_associated.id and qos_info_same_name.id == qos_info_filesystem_associated.id:
            # 同名的qos已经与文件系统关联
            return error_code, error_desc
        if qos_info_filesystem_associated.id and qos_info_same_name.id != qos_info_filesystem_associated.id:
            # 文件系统上当前qos和目标qos不同，重新绑定至目标qos
            log.warn(
                f"File system exist qos id:{qos_info_filesystem_associated.id}, dest qos id:{qos_info_same_name.id}")
            error_code, error_desc = self.delete_converged_qos_association(file_system_name,
                                                                           qos_info_filesystem_associated)
            if error_code != ReturnCode.SUCCESS:
                log.error(
                    f"Update qos fail, delete converged qos association error: {error_code}, error desc: {error_desc}")
                return error_code, error_desc
        error_code, error_desc = self.add_converged_qos_association(file_system_name, qos_info_same_name)
        return error_code, error_desc

    def _build_clone_fs_body(self, file_system_name, source_fs_name, char_set):
        body = {}
        device_details_by_name = DeviceDetails()
        error_code, error_desc = self.query_file_system(source_fs_name, device_details_by_name)
        if error_code != ReturnCode.SUCCESS:
            log.error(f'Failed to create file system body dict parameters to create clone file system, clone fs name: '
                      f'{file_system_name}, source file system name: {source_fs_name}')
            return {}

        device_details = DeviceDetails()
        error_code, error_desc = self.query_file_system_by_id(device_details_by_name.device_id, device_details)
        if error_code != ReturnCode.SUCCESS:
            log.error(f'Failed to create file system body dict parameters to create clone file system, clone fs name: '
                      f'{file_system_name}, source file system name: {source_fs_name}')
            return {}
        body['NAME'] = file_system_name
        body['securityStyle'] = device_details.security_style
        body['ALLOCTYPE'] = device_details.alloc_type
        body['PARENTID'] = device_details.parent_id
        body['ISSHOWSNAPDIR'] = device_details.is_show_snap_dir
        body['ENABLEDEDUP'] = device_details.dedup_enable
        body['ENABLECOMPRESSION'] = device_details.compress_enable
        body['COMPRESSION'] = device_details.compress_algorithm
        body['SECTORSIZE'] = device_details.sector_size
        body['distAlg'] = device_details.dist_alg
        body['skipAffModeCount'] = device_details.skip_affinity_mode_count
        body['unixPermissions'] = device_details.unix_permission
        body['characterSet'] = char_set
        if device_details.total_capacity:
            body['CAPACITY'] = device_details.total_capacity
        if device_details.owning_controller:
            body['OWNINGCONTROLLER'] = device_details.owning_controller
        if device_details.capacity_threshold:
            body['CAPACITYTHRESHOLD'] = device_details.capacity_threshold
        return body

    def _build_create_fs_body(self, source_fs_name, file_system_name, *args):
        pool_id, size_in_mega_bytes, dist_alg, owning_controller_id, capacity_threshold, is_worm_on, char_set, \
        auto_grow_capacity = args
        body = {}
        device_details_check_is_migrating = DeviceDetails()
        if os.getenv('DEPLOY_TYPE') in DEPLOY_TYPE_GROUP_NO_MIGRATING:
            is_migrating = False
        else:
            # 如果状态在迁移，在创建文件系统的时候不指定控制器
            error_code, error_desc = self.check_is_migrating(pool_id, device_details_check_is_migrating)
            if error_code != ReturnCode.SUCCESS:
                log.info(f"Failed to get the migrating status from base")
                return body
            is_migrating = device_details_check_is_migrating.is_migrating
        if is_migrating:
            log.info(f"The system is currently migrating. Will not specify the controller id to create fs")
            owning_controller_id = None
        if not source_fs_name:
            body['NAME'] = file_system_name
            body['securityStyle'] = self._security_style
            body['ALLOCTYPE'] = self._alloc_type
            body['PARENTID'] = pool_id
            body['ISSHOWSNAPDIR'] = self._show_snap_directory
            body['ENABLEDEDUP'] = self._dedup_enable
            body['ENABLECOMPRESSION'] = self._compress_enable
            body['COMPRESSION'] = self._compress_algorithm
            body['SECTORSIZE'] = self._sector_size
            body['characterSet'] = char_set
            body['CAPACITYTHRESHOLD'] = DEFAULT_ALARM_CAPACITY_THRESHOLD
            if size_in_mega_bytes:
                size_in_mega_bytes = NUM_1024 if size_in_mega_bytes < NUM_1024 else size_in_mega_bytes
                body['CAPACITY'] = int(size_in_mega_bytes * NUM_1024 * NUM_1024 / FILE_SYSTEM_SECTOR_IN_BYTES)
            if dist_alg:
                body['distAlg'] = self._directory_distribution_mode
                body['skipAffModeCount'] = self._skip_affinity_mode_count
            if owning_controller_id:
                body['OWNINGCONTROLLER'] = owning_controller_id
            if is_worm_on:
                body['WORMTYPE'] = f'{WORM_TYPE}'
                body['MINPROTECTTIMEUNIT'] = MIN_PROTECT_TIME_UNIT
                body['MAXPROTECTTIMEUNIT'] = WORM_UNIT
                body['DEFPROTECTTIMEUNIT'] = MIN_PROTECT_TIME_UNIT
                body['SUBTYPE'] = f'{WORM_TYPE}'
                body['WORMDEFPROTECTPERIOD'] = WORM_MIN_PROTECT_PERIOD
                body['WORMMAXPROTECTPERIOD'] = WORM_PERIOD
                body['WORMMINPROTECTPERIOD'] = WORM_MIN_PROTECT_PERIOD
                body['WORMAUTODEL'] = False
                body['WORMAUTOLOCK'] = False
            if auto_grow_capacity:
                body['SPACESELFADJUSTINGMODE'] = SpaceSelfAdjustingMode.AUTOMATIC_CAPACITY_EXPANSION
                body['AUTOGROWTHRESHOLDPERCENT'] = AUTO_GROW_THRESHOLD_PERCENT
                body['MAXAUTOSIZE'] = MAX_LOGIC_CAPACITY_FOR_FILESYSTEM
            # OSA适配，将控制参数的开关传入body
            body['source_fs_name_switch'] = bool(source_fs_name)
            body['size_in_mega_bytes_switch'] = bool(size_in_mega_bytes)
            body['dist_alg_switch'] = bool(dist_alg)
            body['owning_controller_id_switch'] = bool(owning_controller_id)
            body['is_worm_on_switch'] = bool(is_worm_on)
            body['auto_grow_capacity_switch'] = bool(auto_grow_capacity)
        else:
            body = self._build_clone_fs_body(file_system_name, source_fs_name, char_set)
        return body

    def _filter_logic_port(self, lif_info: dict, logic_port_list: list):
        logic_port_list.clear()
        for data_info in lif_info.get('data'):
            save_flag = False
            if int(data_info.get('RUNNINGSTATUS', 0)) != LogicPortRunningStatusEnum.LINK_UP or \
                    (int(data_info.get('ROLE', 0)) != LogicPortRoleEnum.PORT_ROLE_SERVICE and
                     int(data_info.get('ROLE', 0)) != LogicPortRoleEnum.PORT_ROLE_MANAGE_SERVICE):
                continue
            if self._nas_share_type == NasShareType.NFS \
                    and (int(data_info.get('SUPPORTPROTOCOL', 0)) in [HostType.SUPPORT_PROTOCOL_NFS,
                                                                      HostType.SUPPORT_PROTOCOL_NFS_CIFS,
                                                                      HostType.ALL]):
                save_flag = True
            elif self._nas_share_type == NasShareType.CIFS \
                    and (int(data_info.get('SUPPORTPROTOCOL', 0)) in [HostType.SUPPORT_PROTOCOL_CIFS,
                                                                      HostType.SUPPORT_PROTOCOL_NFS_CIFS,
                                                                      HostType.ALL]):
                save_flag = True
            elif int(data_info.get('SUPPORTPROTOCOL', 0)) == HostType.DATA_TURBO_PROTOCOL \
                    or int(data_info.get('SUPPORTPROTOCOL', 0)) == HostType.ALL:
                save_flag = True
            if save_flag:
                logic_port_info = LogicPortInfo()
                logic_port_info.ipv4_addr = data_info.get('IPV4ADDR')
                logic_port_info.ipv6_addr = data_info.get('IPV6ADDR')
                logic_port_info.running_status = data_info.get('RUNNINGSTATUS')
                logic_port_info.role = data_info.get('ROLE')
                logic_port_info.support_protocol = data_info.get('SUPPORTPROTOCOL')
                logic_port_info.home_controller_id = data_info.get('HOMECONTROLLERID')
                logic_port_info.port_type = data_info.get('CURRENTPORTTYPE')
                logic_port_list.append(logic_port_info)
        return ReturnCode.SUCCESS, ''

    def _check_and_update_share_info(self, is_ip_sec_open, file_handle_byte_alignment, shared_info):
        # 如果找到了共享信息，共享类型为cifs时，需要更新链路加密开关一致
        if self._nas_share_type == NasShareType.CIFS and shared_info.smb3 != is_ip_sec_open:
            log.info(f'The smb3 value is inconsistent, need to update, is_ip_sec_open:{is_ip_sec_open}.')
            shared_info.smb3 = is_ip_sec_open
            code, desc = self.update_share_info(shared_info)
            if code != ReturnCode.SUCCESS:
                return False
        # 如果找到了共享信息，共享类型为nfs时，需要更新四字节对齐开关一致
        if (self._nas_share_type == NasShareType.NFS and
                shared_info.file_handle_byte_alignment != file_handle_byte_alignment):
            log.info(f'The file_handle_byte_alignment value is inconsistent, need to update,'
                     f'file_handle_byte_alignment:{file_handle_byte_alignment}.')
            shared_info.file_handle_byte_alignment = file_handle_byte_alignment
            code, desc = self.update_share_info(shared_info)
            if code != ReturnCode.SUCCESS:
                return False
        return True

    def _bind_share_info(self, rsp, shared_info):
        for shared_tmp in rsp.get('data'):
            share_path = shared_tmp.get('SHAREPATH').replace('\\', '')
            if "source_policy" not in share_path:
                shared_info.share_id = shared_tmp.get('ID')
                shared_info.share_name = shared_tmp.get('NAME').replace('\\', '')
                shared_info.share_path = share_path
                shared_info.fs_id = shared_tmp.get('FSID')
                if self._nas_share_type == NasShareType.CIFS:
                    shared_info.smb3 = shared_tmp.get('smb3EncryptionEnable')
                return True
        return False

    def _build_share_req_body(self, share_id, user_name_or_ip, related_parameters: dict, body_dict: dict):
        if not related_parameters:
            related_parameters = {}
        if self._nas_share_type == NasShareType.NFS:
            body_dict['NAME'] = f'{user_name_or_ip}'
            body_dict['PARENTID'] = f'{share_id}'
            body_dict['SYNC'] = f'{NfsWriteModeEnum.SYNC}'
            body_dict['ACCESSVAL'] = f'{related_parameters.get("ACCESSVAL", f"{NfsPermissionEnum.READ_WRITE}")}'
            body_dict['ALLSQUASH'] = \
                f'{related_parameters.get("ALLSQUASH", f"{NfsPermissionConstraintEnum.NO_ALL_SQUASH}")}'
            body_dict['ROOTSQUASH'] = \
                f'{related_parameters.get("ROOTSQUASH", f"{NfsRootPermissionConstraintEnum.NO_ROOT_SQUASH}")}'
            body_dict['SECURE'] = f'{related_parameters.get("SECURE", f"{NfsPermissionEnum.READ_WRITE}")}'
            body_dict['ACCESSVAL'] = f'{related_parameters.get("ACCESSVAL", f"{PortVerification.INSECURE}")}'
            body_dict['ACCESSKRB5P'] = f'{related_parameters.get("ACCESSKRB5P", f"{NfsPermissionEnum.FORBIDDEN}")}'
            return ReturnCode.SUCCESS, ''
        elif self._nas_share_type == NasShareType.CIFS:
            body_dict['NAME'] = f'{user_name_or_ip}'
            body_dict['PARENTID'] = f'{share_id}'
            body_dict['DOMANTYPE'] = f'{related_parameters.get("DOMANTYPE", f"{CifsDomainTypeEnum.LOCAL_USER}")}'
            body_dict['PERMISSION'] = f'{related_parameters.get("PERMISSION", f"{CifsPermissionEnum.FULL_CONTROL}")}'
            return ReturnCode.SUCCESS, ''
        else:
            error_code, error_desc = ReturnCode.FAILED, f'The protocol {self._nas_share_type.name} is not supported.'
            log.error(f"Add share client fail, error code: {error_code}, error desc: {error_desc}")
            return error_code, error_desc
