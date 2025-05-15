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
import json
import os
from datetime import datetime

from sqlalchemy import Column, String, Boolean, SmallInteger, DateTime, Time, Integer, ForeignKey, Text, event, JSON
from sqlalchemy.orm import relationship
from sqlalchemy_utils import Timestamp

from app.backup.common.config.base import Base
from app.common.type_decorator.time_type_decorator import TZDateTime

current_path = os.path.abspath(os.path.dirname(__file__))
sla_path = os.path.join(current_path, "data", "sla.json")


class SlaModel(Base):
    __tablename__ = 'sla'

    uuid = Column(String(64), primary_key=True)
    name = Column(String(256), nullable=False, unique=True)
    type = Column(SmallInteger, nullable=False)
    application = Column(String(64), nullable=False)
    user_id = Column(String(64))
    is_global = Column(Boolean, nullable=False)
    created_time = Column(DateTime, nullable=False, default=datetime.now)
    policy_list = relationship('PolicyModel', lazy="joined", cascade="save-update, delete, merge")


class PolicyModel(Base):
    __tablename__ = 'policy'

    uuid = Column(String(64), primary_key=True)
    sla_id = Column(String(64), ForeignKey(SlaModel.uuid), nullable=False)
    name = Column(String(128), nullable=False)
    type = Column(String(16), nullable=False)
    env_type = Column(String(16))
    object_type = Column(String(16))
    action = Column(String(32))
    active = Column(Boolean, nullable=False, default=True)
    protection_mode = Column(String(16))
    ext_parameters = Column(JSON)
    retention = relationship("RetentionModel", lazy="joined", uselist=False, cascade="save-update, delete, merge")
    schedule = relationship("ScheduleModel", lazy="joined", uselist=False, cascade="save-update, delete, merge")


class RetentionModel(Base):
    __tablename__ = 'retention'

    policy_id = Column(String(64), ForeignKey(PolicyModel.uuid), primary_key=True)
    retention_type = Column(SmallInteger, nullable=False)
    retention_duration = Column(Integer)
    duration_unit = Column(String(4))
    daily_copies = Column(Integer)
    weekly_copies = Column(Integer)
    monthly_copies = Column(Integer)
    yearly_copies = Column(Integer)


class ScheduleModel(Timestamp, Base):
    __tablename__ = 'schedule'

    policy_id = Column(String(64), ForeignKey(PolicyModel.uuid), primary_key=True)
    trigger = Column(SmallInteger, nullable=False)
    interval = Column(Integer)
    interval_unit = Column(String(4))
    start_time = Column(TZDateTime)
    window_start = Column(TZDateTime)
    window_end = Column(TZDateTime)
    days_of_week = Column(String(128))
    days_of_month = Column(String(128))
    days_of_year = Column(String(DateTime))
    trigger_action = Column(String(16))
    ext_parameters = Column(Text)


def init_sla_data(target, connection, **kw):
    with open(sla_path, 'r') as file:
        data = json.load(file)
        connection.execute(target.insert(), *data["sla_list"])


event.listen(SlaModel.__table__, "after_create", init_sla_data)


def init_policy_data(target, connection, **kw):
    with open(sla_path, 'r') as file:
        data = json.load(file)
        connection.execute(target.insert(), *data["policy_list"])


event.listen(PolicyModel.__table__, "after_create", init_policy_data)


def init_retention_data(target, connection, **kw):
    with open(sla_path, 'r') as file:
        data = json.load(file)
        connection.execute(target.insert(), *data["retention_list"])


event.listen(RetentionModel.__table__, "after_create", init_retention_data)


def init_schedule_data(target, connection, **kw):
    with open(sla_path, 'r') as file:
        data = json.load(file)
        connection.execute(target.insert(), *data["schedule_list"])


event.listen(ScheduleModel.__table__, "after_create", init_schedule_data)
