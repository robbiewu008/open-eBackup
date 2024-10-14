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
from kubernetes import client, config
from kubernetes.watch import Watch
from kubernetes.client import ApiClient
from kubernetes.client.exceptions import ApiException

from public_cbb.log.logger import get_logger
from public_cbb.security.anonym_utils.anonymity import Anonymity

logger = get_logger()


class KubernetesApi:
    def __init__(self, in_cluster=True):
        # it's useful for K8S backup
        self.in_cluster = in_cluster

    def get_api_method(self, api_class_name, api_method_name):
        api_class = getattr(client, api_class_name)
        return getattr(api_class(api_client=self._get_api_client()), api_method_name)

    def register_watch(self, api_class_name, api_method_name, handle_event_func, **kwargs):
        logger.info("Register watching job.")
        watch = Watch()
        func = self.get_api_method(api_class_name, api_method_name)
        try:
            for event in watch.stream(func, **kwargs):
                handle_event_func(event)
        except ApiException as ex:
            logger.error("ApiException when watch pod status, %s", Anonymity.process(str(ex)))

    def _get_api_client(self):
        if self.in_cluster:
            config.load_incluster_config()
        return ApiClient()
