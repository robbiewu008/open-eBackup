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

from openGauss.common.base_cmd import BaseCmd
from openGauss.common.const import logger


class Database:
    """查询实例下的数据库"""

    def __init__(self, db_user, db_port, node_ip="localhost", db_env_path=""):
        self.db_user = db_user
        self.db_port = db_port
        self.db_ip = node_ip
        self.env_path = db_env_path

    def show_databases(self):
        db_names = self._show_databases_by_cmd()
        return [db.strip(" ") for db in db_names]

    def _show_databases_by_cmd(self):
        base_cmd = BaseCmd(self.db_user, self.env_path)
        ret, cont = base_cmd.show_all_databases(self.db_port)
        if not ret:
            error_message = f"Get database failed or database is not exist in {self.db_port}"
            logger.error(error_message)
            return []
        # 获取实例节点下数据库名
        return [line for line in cont.split("\n") if line][2:-1]
