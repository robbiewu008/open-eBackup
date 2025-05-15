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

from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient, decode_response_data, parse_response_data
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException

LOGGER = logger.get_logger(__name__)


def get_system_base_by_url(url):
    LOGGER.debug(f'invoke api request url is {url}')
    response = SystemBaseHttpsClient().request("GET", url)
    if response.status == HTTPStatus.OK:
        return parse_response_data(response.data)

    LOGGER.error(f"get error url: {url}")
    raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                               error_message="invoke api failed or timeout")


class SystemBaseClient(object):

    @staticmethod
    def query_user(user_id):
        url = f'/v1/internal/users/{user_id}'
        LOGGER.info(f'invoke api to create jon, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            LOGGER.error(
                f'Failed to query resource info, resource id is {user_id}')

    @staticmethod
    def query_sub_resource(resource_id):
        pass

    @staticmethod
    def encrypt(plaintext: str):
        url = f'/v1/kms/encrypt/'
        encrypt_body = {"plaintext": plaintext}
        LOGGER.info(f'invoke api to en_crypt, request url is {url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(encrypt_body))
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            LOGGER.error(
                f'Failed to en_crypt plaintext, url is {url}')
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, parameters=[])

    @staticmethod
    def get_role_list_by_domain_id_from_base(domain_id: str):
        url = f'/v1/internal/users/roles/'+domain_id
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            LOGGER.error(
                f'Failed to en_crypt plaintext, url is {url}')
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, parameters=[])

    @staticmethod
    def decrypt(ciphertext: str):
        url = f'/v1/kms/decrypt/'
        decrypt_body = {"ciphertext": ciphertext}
        LOGGER.info(f'invoke api to de_crypt, request url is {url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(decrypt_body))
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            LOGGER.error(f'Failed to de_crypt ciphertext, url is {url}')
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, parameters=[])

    @staticmethod
    def get_auth_certificate():
        url = f'/v1/auth/certificate'
        LOGGER.debug(f'invoke api to get auth certificate, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status == HTTPStatus.OK:
            return decode_response_data(response.data)
        else:
            LOGGER.error('Failed to get auth certificate')
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, parameters=[])

    @staticmethod
    def get_time_zone():
        url = f'/v1/system/internal/timezone'
        LOGGER.debug(f'invoke api to get time zone, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status == HTTPStatus.OK:
            return decode_response_data(response.data)
        else:
            LOGGER.error(
                f'Failed to get time zone info, url is {url}')
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, parameters=[])

    @staticmethod
    def query_filesystem(filesystem_id: str):
        """
        :param filesystem_id:
        :return: {"id": "123","name": "xxxx","remoteReplicationIds": ["123","123"]}
        """
        if filesystem_id:
            url = f'/v1/internal/local-storage/filesystem/{filesystem_id}'
            return get_system_base_by_url(url=url)
        LOGGER.debug("filesystem_id is none")

    @staticmethod
    def query_replication_pair(replication_pair_id: str):
        """
        :param replication_pair_id:
        :return: {"id": null,"replicationModel": 0,"primary": false}
        """
        if replication_pair_id:
            url = f'/v1/internal/local-storage/replicationpair/{replication_pair_id}'
            return get_system_base_by_url(url=url)
        LOGGER.debug("replication_pair_id is none")

    @staticmethod
    def query_hyper_metro_pair(hyper_metro_pair_id: str):
        """
        :param hyper_metro_pair_id:
        :return: {"id": null,"replicationModel": 0,"primary": false}
        """
        url = f'/v1/internal/local-storage/hypermetropair/{hyper_metro_pair_id}'
        return get_system_base_by_url(url=url)

    @staticmethod
    def query_local_storage_fssnapshot(snapshot_id: str, tenant_id: str):
        """
        :param snapshot_id 快照id tenant_id: 租户id
        :return: {"id": 快照id,"name": 快照名称,"isInProtectionPeriod": 是否为安全快照, "isInProtectionPeriod": 是否在保护期}
        """
        url = f'/v1/internal/local-storage/fssnapshot/{snapshot_id}'

        LOGGER.debug(f'invoke api request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url,
                                                   fields={"vStoreId": tenant_id})
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        LOGGER.error(f"get error url: {url} status: {response.status} error: {parse_response_data(response.data)}")
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                   error_message="invoke api failed or timeout")

    @staticmethod
    def query_local_storage_cdp(snapshot_id: str, tenant_id: str):
        """
        :param snapshot_id 快照id tenant_id: 租户id
        :return: {"id": 快照id,"name": 快照名称,"secureSnapEnabled": 是否为安全快照, "isInProtectionPeriod": 是否在保护期}
        """
        url = f'/v1/internal/local-storage/cdp/{snapshot_id}'

        LOGGER.debug(f'invoke api request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url,
                                                   fields={"vStoreId": tenant_id})
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        LOGGER.error(f"get error url: {url} status: {response.status} error: {parse_response_data(response.data)}")
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                   error_message="invoke api failed or timeout")

    @staticmethod
    def get_fs_snapshot_by_names(file_system_name: str, snapshot_name: str):
        """
        :param file_system_name: 系统文件名称 snapshot_name: 快照名称
        :return: {"id": 快照id,"name": 快照名称,"isInProtectionPeriod": 是否为安全快照, "isInProtectionPeriod": 是否在保护期}
        """
        url = f'/v1/internal/local-storage/fssnapshot/info'
        LOGGER.debug(f'invoke api request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url,
                                                   fields={"fileSystemName": file_system_name,
                                                           "snapshotName": snapshot_name})
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        LOGGER.error(f"get error url: {url} status: {response.status} error: {parse_response_data(response.data)}")
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                   error_message="invoke api failed or timeout")

    @staticmethod
    def query_remote_storage_fssnapshot(snapshot_id: str, tenant_id: str, device_id: str):
        """
        :param snapshot_id 快照id tenant_id: 租户id
        :return: {"id": 快照id,"name": 快照名称,"isInProtectionPeriod": 是否为安全快照, "isInProtectionPeriod": 是否在保护期}
        """
        url = f'/v1/internal/plugins/storage/snapshots/{snapshot_id}'

        LOGGER.debug(f'invoke api request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url,
                                                   fields={"vStoreId": tenant_id, "deviceId": device_id})
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        LOGGER.error(f"get error url: {url} status: {response.status} error: {parse_response_data(response.data)}")
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                   error_message="invoke api failed or timeout")

    @staticmethod
    def query_remote_storage_filesystem(device_id: str, filesystem_id: str):
        """
        :param filesystem_id:
        :return: {"id": "123","name": "xxxx","HYPERMETROPAIRIDS": ["123","123"]}
        """
        if filesystem_id and device_id:
            url = f'/v1/internal/plugins/storage/filesystems/{filesystem_id}'
            LOGGER.debug(f'invoke api request url is {url}')
            response = SystemBaseHttpsClient().request("GET", url, fields={"deviceId": device_id})
            if response.status == HTTPStatus.OK:
                return parse_response_data(response.data)
            LOGGER.error(f"get error url: {url} status: {response.status} error: {parse_response_data(response.data)}")
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="invoke api failed or timeout")
        LOGGER.debug("filesystem_id or device_id is none")
        return {}

    @staticmethod
    def query_remote_storage_hyper_metro_pair(device_id: str, hyper_metro_pair_id: str):
        """
        :param hyper_metro_pair_id:
        :return: {"id": null,"replicationModel": 0,"primary": false}
        """
        if hyper_metro_pair_id and device_id:
            url = f'/v1/internal/plugins/storage/hypermetropair/{hyper_metro_pair_id}'
            LOGGER.debug(f'invoke api request url is {url}')
            response = SystemBaseHttpsClient().request("GET", url, fields={"deviceId": device_id})
            if response.status == HTTPStatus.OK:
                return parse_response_data(response.data)
            LOGGER.error(f"get error url: {url} status: {response.status} error: {parse_response_data(response.data)}")
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="invoke api failed or timeout")
        LOGGER.debug("hyper_metro_pair_id or device_id is none")
        return {}
