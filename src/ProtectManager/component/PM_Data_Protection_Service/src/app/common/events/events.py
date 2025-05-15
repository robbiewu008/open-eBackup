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
import pkg_resources
from app.common import logger
from app.common.event_messages.event import EventMessage

LOG = logger.get_logger(__name__)


def get_es_event_messages_version():
    return pkg_resources.get_distribution('es_event_messages').version


def is_subclass_of_event_message(obj):
    try:
        LOG.info("obj is class.")
        return issubclass(obj, EventMessage)
    except TypeError:
        LOG.info("obj is instance.")
        return isinstance(obj, EventMessage)
