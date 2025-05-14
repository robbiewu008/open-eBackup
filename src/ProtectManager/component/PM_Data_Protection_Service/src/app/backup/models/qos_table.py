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
from sqlalchemy import Column, String, DateTime, Integer

from app.backup.common.config.base import Base
from app.backup.schemas.qos import QosRes


class QosTable(Base):
    __tablename__ = 'qos'

    uuid = Column(String(64), primary_key=True)
    name = Column(String(256), nullable=False, unique=True)
    speed_limit = Column(Integer, nullable=False)
    created_time = Column(DateTime, nullable=False)
    user_id = Column(String(255), nullable=True)
    description = Column(String(1024), nullable=True)

    __filter_fields__ = ["%name%"]
    __mapper_args__ = {
        'polymorphic_identity': __tablename__
    }

    __relation__ = {
        "schema": QosRes,
        "order_by": ["-created_time"],
    }
