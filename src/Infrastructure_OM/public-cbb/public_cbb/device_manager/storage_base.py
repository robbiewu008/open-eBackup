#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import json
import os
import threading
import time

import urllib3
from starlette import status

from public_cbb.communication.rest.https_request import HttpRequest, RequestUtils
from public_cbb.config.global_config import get_settings
from public_cbb.log.logger import get_logger
from public_cbb.device_manager.device_info import DeviceInfo, OsaDeviceInfo
from public_cbb.device_manager.http_params import SessionInfo
from public_cbb.device_manager.constants import (
    StorageErrorCode,
    ReturnCode,
    PROTECT_ENGINE_E_DMA_DOMAIN,
    PROTECT_ENGINE_E_DMA_PORT,
    MAX_STORAGE_RETRIES,
    StorageNoNeedRetryErrorCode,
    STORAGE_RETRY_INTERVAL_TIME_SEC,
    DirectoryDistributionModeEnum
)
from public_cbb.security.pwd_manage.pwd_manage import clear
from public_cbb.security.anonym_utils.anonymity import Anonymity
from public_cbb.utils.decorator import retry

log = get_logger()
urllib3.disable_warnings()
storage_session_lock = threading.Lock()


class StorageBase:
    def __init__(self, device_info: DeviceInfo):
        self.environment = os.getenv('ENVIRONMENT')
        self.init_success = False
        self.has_logged = False
        self._storage_pool_id = device_info.pool_id
        self._dedup_enable = device_info.dedup_enable
        self._compress_enable = device_info.compress_enable
        self._compress_algorithm = device_info.compress_algorithm
        self._sector_size = device_info.sector_size
        self._show_snap_directory = device_info.show_snap_directory
        self._alloc_type = device_info.alloc_type
        self._security_style = device_info.security_style
        self._directory_distribution_mode = device_info.directory_distribution_mode
        self._skip_affinity_mode_count = device_info.skip_affinity_mode_count
        self._temp_host_id = ''
        self._retry_times = MAX_STORAGE_RETRIES
        self._retry_interval_time = STORAGE_RETRY_INTERVAL_TIME_SEC
        self.device_id = device_info.device_id
        self.username = device_info.username
        self.osa_port = '30173'
        self.osa_host = 'dme-openstorageapi'
        self.init_success = True

    def __del__(self):
        if self.environment != 'Dorado' or self.has_logged:
            log.info('The storage session has been logged out.')
        self.init_success = False

    @staticmethod
    def _check_response(rsp):
        if not isinstance(rsp, Exception):
            if rsp.get("error", {}).get("code", None) is None and rsp.get("result", {}).get("code", None) is None:
                error_desc = f'Storage response json format error, without error or error.code'
                return ReturnCode.FAILED, error_desc
            else:
                error_code = ReturnCode.SUCCESS
                error_desc = ''
                if rsp.get("error", {}).get('code'):
                    error_code = int(rsp.get('error').get('code'))
                    error_desc = rsp.get('error').get('description')
                elif rsp.get("result", {}).get('code'):
                    error_code = int(rsp.get('result').get('code'))
                    error_desc = rsp.get('result').get('description')
                return error_code, error_desc
        else:
            return ReturnCode.FAILED, f'Storage response exception!'

    @retry(Exception, get_settings().INFRA_RETRY_TIMES_FOR_NETWORK_ERROR,
           get_settings().INFRA_RETRY_INTERVAL_FOR_NETWORK_ERROR)
    def _send_request_in_storage(self, req: HttpRequest, suffix_temp, device_info, url=None):
        req.https = True
        # 加微服务的证书
        req.verify = True
        req.headers = {'Content-Type': 'application/json'}
        if device_info:
            if not url:
                req.suffix = f'/v1/internal/deviceManager/rest/{device_info.device_id}{suffix_temp}'
                if "?" in suffix_temp:
                    req.suffix += '&'
                else:
                    req.suffix += '?'
                req.suffix += f'username={device_info.username}'
            else:
                req.suffix = url
        request = RequestUtils(req)
        status_code, rsp = request.send_request()
        if status_code != status.HTTP_200_OK:
            log.error(f'Send request in storage failed, status code:{status_code}.')
            return ''
        error_code, error_desc = self._check_response(rsp)
        unauthorized_error_code = {
            StorageErrorCode.UNAUTH, StorageErrorCode.NO_USER_PERMISSION,
            StorageErrorCode.AUTH_IP_INCONSISTENCY, StorageErrorCode.SESSION_INVALIDATED
        }
        if error_code and error_code in unauthorized_error_code:
            log.info(f'Unauthorized REST.')
        return rsp

    def set_retry_attr(self, retry_times, retry_interval_time):
        self._retry_times = retry_times
        self._retry_interval_time = retry_interval_time

    def set_deduption(self, flag):
        self._dedup_enable = flag

    def set_compression(self, flag):
        self._compress_enable = flag

    def get_compression(self):
        return self._compress_enable

    def set_dist_alg(self, dist_alg: DirectoryDistributionModeEnum):
        self._directory_distribution_mode = dist_alg

    def set_skip_affinity_mode_count(self, skip_affinity_mode_count: int):
        self._skip_affinity_mode_count = skip_affinity_mode_count

    def _send_request(self, req: HttpRequest, lock_session=False, url=None):
        # 增加storage消息重试
        rsp = {}
        retry_num = 0
        suffix_temp = req.suffix
        req.host = self.osa_host
        req.port = self.osa_port
        osa_device_info = OsaDeviceInfo(device_id=self.device_id, username=self.username)
        log.info(f"Device id: {self.device_id}, username: {self.username}, host:{self.osa_host}, port:{self.osa_port}")

        while retry_num < self._retry_times:
            # 增加特定资源访问锁
            if lock_session:
                storage_session_lock.acquire()
            network_error = False
            try:
                rsp = self._send_request_in_storage(req=req, suffix_temp=suffix_temp, device_info=osa_device_info,
                                                    url=url)
            except Exception as e:
                log.error(f'Network error, will try. ')
                network_error = True
                rsp = e
            if lock_session:
                storage_session_lock.release()
            if network_error:
                retry_num += 1
                continue
            # 如果是网络原因，http.session已经设置了重试次数。
            # 此处检测storage返回错误码时，根据该错误码判断是否需要重试
            error_code, error_desc = self._check_response(rsp)
            if (error_code == ReturnCode.SUCCESS) or (error_code in StorageNoNeedRetryErrorCode) or \
                    error_code == ReturnCode.FAILED:
                # storage明确返回SUCCESS、FAILED、或不需要重试的错误码，则退出重试机制
                break
            time.sleep(self._retry_interval_time)
            retry_num += 1
            log.debug(f'Already send request to storage for try num:{retry_num}')
        return rsp
