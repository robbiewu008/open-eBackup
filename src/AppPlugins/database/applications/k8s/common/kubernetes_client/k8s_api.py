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

import re
import os
import stat
from functools import wraps
import threading
import base64
import urllib3
from urllib3.exceptions import MaxRetryError, NewConnectionError
from common.const import ParamConstant
from kubernetes import client, config
from kubernetes.client import ApiClient
from k8s.common.kubernetes_client.struct import ClusterAuthentication, AuthType
from k8s.logger import log


class InitKubernetesApiError(Exception):
    def __init__(self):
        super().__init__()
        log.error('Get kubernetes client failed.')


class AutoLock:
    def __init__(self):
        self._lock = threading.Lock()

    def __enter__(self):
        log.debug('wait for available lock')
        self._lock.acquire()

    def __exit__(self, exc_type, exc_val, exc_tb):
        log.debug('unlock')
        self._lock.release()


def api_exception_handler(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except Exception as msg:
            log.exception(f"Exec func({func.__name__}) failed.")
            raise InitKubernetesApiError from msg

    return wrapper


def singleton(cls):
    _instance = {}
    count = 0

    @wraps(cls)
    def _singleton(*args, **kargs):
        if "no_deco" in kargs.keys() and kargs["no_deco"] is True:
            return cls(*args, **kargs)
        nonlocal count
        if cls not in _instance:
            _instance[cls] = cls(*args, **kargs)
        count += 1
        return _instance[cls]

    return _singleton


@singleton
class ClientCenter:
    def __init__(self, *args, **kargs):
        self.client_map = dict()

    @staticmethod
    def _parse_ip_from_config(conf: dict):
        if not isinstance(conf, dict):
            return ""
        local_pattern = (
            "(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)[.]){3}"
            "(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
        )
        try:
            match_result = re.findall(
                local_pattern,
                str(conf["clusters"][0]["cluster"]["server"])
            )
        except BaseException as msg:
            log.error(msg, exc_info=True)
            return ""
        if len(match_result) == 1:
            log.debug(f"find ip: {match_result[0]}")
            return str(match_result[0])
        return ""

    @staticmethod
    def _get_python_client_api_name(k8s_api_name, api_ver):
        if k8s_api_name.endswith(".k8s.io"):
            api_name = k8s_api_name[:-len(".k8s.io")]
        else:
            api_name = k8s_api_name
        api_name = (
                api_name.capitalize().replace("Rbac.auth", "RbacAuth")
                + api_ver.capitalize()
                + "Api"
        )
        return api_name

    @staticmethod
    def _generate_custom_api_name_map(
            api_name_after_combined,
            k8s_client,
            api_name,
            api_ver,
            api_groups_map
    ):
        try:
            api_base_class = getattr(client, api_name_after_combined)
            if not api_base_class:
                api_resources = api_base_class(api_client=k8s_client).get_api_resources()
            api_groups_map[api_name_after_combined] = [api_name, api_ver]
        except Exception:
            # if resource of a api is None, it maybe a custom object api
            # this API does not have a standard get_resources method
            # probably a CRD api, adding it to the custom_api_name_map
            pass

    @staticmethod
    def _get_client_from_authentication(
            ip: str,
            authentication: ClusterAuthentication
    ):
        if authentication.auth_type == AuthType.CONFIGFILE:
            configuration = client.Configuration()
            configuration.verify_ssl = authentication.is_verify_ssl == '1'
            config.load_kube_config_from_dict(authentication.kube_config,
                                              client_configuration=configuration)
            client.Configuration.set_default(configuration)
            log.debug("end config file!")
        else:
            configuration = client.Configuration()
            configuration.host = f'https://{authentication.token.address}:{authentication.token.port}'
            configuration.api_key = {"authorization": "Bearer " + authentication.token.token_info}
            if authentication.is_verify_ssl == "0":
                configuration.verify_ssl = False
                client.Configuration.set_default(configuration)
                k8s_client = ApiClient()
                return k8s_client
            configuration.verify_ssl = True
            cert = base64.b64decode(authentication.token.certificateAuthorityData).decode('utf-8')
            input_file_path = os.path.join(ParamConstant.PARAM_FILE_PATH,
                                           ParamConstant.INPUT_FILE_PREFFIX + authentication.id + ".pem")
            flags = os.O_WRONLY | os.O_CREAT
            modes = stat.S_IRUSR | stat.S_IWUSR | stat.S_IRGRP | stat.S_IROTH
            with os.fdopen(os.open(input_file_path, flags, modes), 'w') as file_stream:
                log.info("start write file")
                file_stream.truncate(0)
                file_stream.write(cert)
            log.info("set ssl file success!")
            configuration.ssl_ca_cert = input_file_path
            client.Configuration.set_default(configuration)
        k8s_client = ApiClient()
        return k8s_client

    def get_client(self, authentication: ClusterAuthentication):
        log.debug(f'auth_type: {authentication.auth_type}')
        if authentication.auth_type == AuthType.CONFIGFILE:
            ip = self._parse_ip_from_config(authentication.kube_config)
        else:
            ip = authentication.token.address
        if ip == "":
            log.error("Get ip from key failed")
            raise InitKubernetesApiError
        return self._get_client_from_client_center(ip, authentication)

    def _find_prefer_version_for_client(self, k8s_client):
        prefer_api_versions = set()
        preferred_apis = dict()
        core_api_ver = client.CoreApi(
            api_client=k8s_client
        ).get_api_versions(_request_timeout=5).versions
        preferred_apis["core"] = core_api_ver[0]
        for api in client.ApisApi(
                api_client=k8s_client
        ).get_api_versions(_request_timeout=10).groups:
            for ver in api.versions:
                if len(api.versions) == 1:
                    preferred_apis[api.name] = ver.version
                elif ver.version == api.preferred_version.version \
                        and len(api.versions) > 1:
                    preferred_apis[api.name] = ver.version
        custom_api_name_map = dict()
        api_groups_map = dict()
        for api_name, api_ver in preferred_apis.items():
            api_name_after_combined = self._get_python_client_api_name(api_name, api_ver)
            try:
                self._generate_custom_api_name_map(
                    api_name_after_combined,
                    k8s_client,
                    api_name,
                    api_ver,
                    api_groups_map
                )
                custom_api_name_map[api_name] = api_ver
            except AttributeError as e:
                log.error(
                    f"failed to call get_api_resources for: {api_name_after_combined}, exception: {str(e)}"
                    "This error may occur for new APIs and Custom Resources APIs."
                )
            prefer_api_versions.add(api_name_after_combined)
        prefer_api_versions.add("CustomObjectsApi")
        prefer_api_versions.add("VersionApi")
        prefer_api_versions.add("BatchV1beta1Api")
        return prefer_api_versions, custom_api_name_map, api_groups_map

    def _get_client_from_client_center(
            self,
            ip: str,
            authentication: ClusterAuthentication
    ):
        with AutoLock():
            log.debug(f"get lock")
            if ip in self.client_map.keys():
                log.debug(f"find client with ip: {ip}")
                return self.client_map[ip]
            else:
                log.info(f"can not find client with ip: {ip}, create a new one")
                try:
                    k8s_client = self._get_client_from_authentication(ip, authentication)
                except NewConnectionError as e:
                    log.error(e, exc_info=True)
                    raise urllib3.exceptions.HTTPError from e
                except MaxRetryError as e:
                    log.error(e, exc_info=True)
                    raise urllib3.exceptions.HTTPError from e
                except config.ConfigException as e:
                    log.error(e, exc_info=True)
                    raise InitKubernetesApiError from e
                if k8s_client is None:
                    raise InitKubernetesApiError
                temp_client = KubeClientWithPreferVersions()
                try:
                    api_versions, custom_api_name_map, api_groups_map = \
                        self._find_prefer_version_for_client(k8s_client)
                except urllib3.exceptions.HTTPError as e:
                    log.error(f"GetClientFromClientCenter:{e}")
                    raise urllib3.exceptions.HTTPError from e
                except urllib3.exceptions.MaxRetryError as e:
                    log.error(f"GetClientFromClientCenter:{e}")
                    raise urllib3.exceptions.HTTPError from e
                temp_client.set_client(k8s_client)
                temp_client.set_prefer_versions(api_versions)
                temp_client.set_custom_api_name_map(custom_api_name_map)
                temp_client.set_api_groups_map(api_groups_map)
                self.client_map[ip] = temp_client
                return temp_client


class KubeClientWithPreferVersions:
    def __init__(self):
        self._client = ApiClient()
        self._prefer_versions = []
        self._custom_api_name_map = dict()
        self._api_groups_map = dict()

    def set_client(self, client_):
        self._client = client_

    def get_client(self):
        return self._client

    def set_prefer_versions(self, prefer_versions):
        self._prefer_versions = prefer_versions

    def get_prefer_versions(self):
        return self._prefer_versions

    def set_custom_api_name_map(self, custom_api_name_map):
        self._custom_api_name_map = custom_api_name_map

    def get_custom_api_name_map(self):
        return self._custom_api_name_map

    def set_api_groups_map(self, api_groups_map):
        self._api_groups_map = api_groups_map

    def get_api_groups_map(self):
        return self._api_groups_map


class ApiBase:
    POSTFIX = ".*Api"
    AppsV1Api = "AppsV1Api"

    def __init__(
            self,
            cluster_authentication: ClusterAuthentication,
            pattern: str
    ):
        attribute_name = ""
        log.debug(f"pattern={pattern}")
        self.k8s_client = ClientCenter().get_client(cluster_authentication)
        if self.k8s_client is None:
            raise InitKubernetesApiError
        for api in self.k8s_client.get_prefer_versions():
            if re.match(pattern, api) is not None:
                log.debug(f"find api! Api={api}")
                attribute_name = api
                break
        if not attribute_name:
            raise InitKubernetesApiError
        self.api_base_class = getattr(client, attribute_name)
        log.debug("ApiBase Succeed!")

    def get_api_attr(self, api_name):
        return getattr(
            self.api_base_class(api_client=self.k8s_client.get_client()),
            api_name
        )
