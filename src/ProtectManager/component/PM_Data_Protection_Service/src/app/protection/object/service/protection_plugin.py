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
from typing import List

from app.protection.object.common.protection_enums import ResourceFilter


class ProtectionPlugin:
    def do_convert_extend_parameter(self, filter_list: List[ResourceFilter], resource, extend_parameter):
        pass

    def build_protection_object(self, resource_info, sla, ext_params):
        pass

    def query_sub_resources(self, resource_info):
        pass

    def query_sub_resources_by_obj(self, obj):
        pass

    def query_sub_resources_only_vm(self, obj):
        pass

    def build_task_list(self, sla, resource_id, projected_object, execute_req):
        pass

    def build_ext_parameters(self, ext_parameters):
        pass
