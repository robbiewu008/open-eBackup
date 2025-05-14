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
from sqlalchemy import Column, String, BigInteger, Integer, Text, BOOLEAN

from app.base.db_base import Base


class JobTable(Base):
    __tablename__ = 't_job'

    job_id = Column(String(64), primary_key=True)
    user_id = Column(String(256))
    type = Column(String(128), nullable=False)
    progress = Column(Integer, nullable=False)
    start_time = Column(BigInteger, nullable=False)
    end_time = Column(BigInteger)
    last_update_time = Column(BigInteger)
    status = Column(String(32))
    parent_id = Column(String(64))
    speed = Column(String(128))
    detail = Column(String(256))
    detail_para = Column(String(256))
    extend_str = Column(Text)
    associative_id = Column(String(64))
    associative_type = Column(Integer)
    source_id = Column(String(64))
    source_name = Column(String(512))
    source_sub_type = Column(String(128))
    source_location = Column(String(1024))
    target_name = Column(String(512))
    target_location = Column(String(1024))
    copy_id = Column(String(64))
    copy_time = Column(BigInteger)
    enable_stop = Column(BOOLEAN)
    is_system = Column(BOOLEAN)
    is_visible = Column(BOOLEAN)
    request_id = Column(String(128))
    additional_status = Column(String(128))
    message = Column(Text)
    data = Column(Text)
    source_type = Column(String(128))
    log_levels = Column(Integer, default=0)
    device_esn = Column(String(256))
    sla_id = Column(String(64))
    exercise_id = Column(String(64), default='')
    exercise_job_id = Column(String(64), default='')
    storage_unit_id = Column(String(256), default='')
    resource_group_id = Column(String(64), default='')
    group_backup_job_id = Column(String(64), default='')
    mark = Column(Text)
    mark_status = Column(String(64), default='0')

    def as_dict(self):
        job_dict = {col.name: getattr(self, col.name) for col in self.__table__.columns}
        return job_dict
