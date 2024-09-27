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

from dameng.resource.damengsource import DamengSource

from common.common import exter_attack
from common.logger import Logger

LOGGER = Logger().get_logger("dameng.log")


class QueryJobPermission(DamengSource):

    @exter_attack
    def get_resource(self, param_info_):
        result_info = {
            "user": "",
            "group": "",
            "fileMode": ""
        }
        result_type, user, group = self.discover_application()
        LOGGER.info(f"QueryJobPermission-user:{user} group:{group}.")
        if result_type:
            result_info["user"] = user
            result_info["group"] = group
            result_info["fileMode"] = "0755"
        LOGGER.info("Get QueryJobPermission_result_info.")
        return result_info
