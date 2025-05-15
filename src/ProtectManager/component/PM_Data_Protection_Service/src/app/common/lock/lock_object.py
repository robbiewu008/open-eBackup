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
from sqlalchemy import Column, String, DateTime

from app.base.db_base import Base


class LockObject(Base):
    __tablename__ = 't_distributed_lock'

    id = Column(String(64), primary_key=True)
    key = Column(String(256), nullable=False, index=True, unique=True)
    description = Column(String(1024))
    lock_time = Column(DateTime, nullable=False)
    unlock_time = Column(DateTime, nullable=False)
    owner = Column(String(256))
