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
from sqlalchemy import Column, String

from app.base.db_base import Base
from app.common.logger import get_logger

log = get_logger(__name__)


class RepSlaUser(Base):
    __tablename__ = 't_external_users'
    uuid = Column(String(256), primary_key=True)
    user_id = Column(String(256))
    sla_id = Column(String(256))
    policy_id = Column(String(256))
    username = Column(String(256))
    password = Column(String(2048))
    user_type = Column(String(256))
