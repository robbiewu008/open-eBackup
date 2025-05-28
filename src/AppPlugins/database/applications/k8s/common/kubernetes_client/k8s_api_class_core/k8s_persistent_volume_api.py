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

from k8s.common.kubernetes_client.k8s_api import ApiBase, api_exception_handler
from k8s.logger import log


class ResourcePersistentVolume(ApiBase):
    def __init__(self, cluster_authentication):
        super().__init__(cluster_authentication, f'Core{self.POSTFIX}')

    @api_exception_handler
    def read(self, name, **kwargs):
        if 'namespace' in kwargs.keys():
            log.error('namespace should not contained')
            kwargs.pop('namespace')
        api_method = self.get_api_attr('read_persistent_volume')
        return api_method(name, **kwargs)

    @api_exception_handler
    def list(self, **kwargs):
        if 'namespace' in kwargs.keys():
            log.error('namespace should not contained')
            kwargs.pop('namespace')
        api_method = self.get_api_attr('list_persistent_volume')
        return api_method(**kwargs)
