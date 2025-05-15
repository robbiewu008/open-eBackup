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
from app.common.logger import get_logger

log = get_logger(__name__)


def some_time_value(timestamp):
    t = None
    time_type = type(timestamp)
    log.debug(f'got timestamp={timestamp}, type: {time_type}.')
    try:
        if time_type == str:
            t = float(timestamp)
            if str.isdigit(timestamp):
                t = abs(int(t / 1000000))
            else:
                t = abs(int(timestamp))
        elif time_type == float:
            t = abs(int(timestamp))
        elif time_type == int:
            t = abs(int(timestamp / 1000000))
        else:
            t = 585225380  # some default value
    except ValueError:
        log.debug(f'Value-Exception , set copyct to dummy.')
        t = 585225380  # some default value
    finally:
        pass
    log.debug(f'copyct value will be {t}.')
    return t


class EsCatalogDocument:
    def __init__(
        self,
        snap_id: str,
        policy_id: str,
        p_obj: dict,
        res_id: str,
        action: str,
        path_db: str,
        timestamp,
    ):
        self.copyid = snap_id
        self.slaid = policy_id if isinstance(policy_id, str) else ''
        self.slaname = "dummy_policy_name"
        self.copyct = some_time_value(timestamp)
        self.copysize = 1000
        self.resname = 'future use'
        self.action = action
        self.copylocalid = p_obj['object_type'] if 'object_type' in p_obj else ''
        self.reshostname = path_db
        self.restype = p_obj['env_type'] if 'env_type' in p_obj else ''
        self.resid = res_id
        self.isuse = False
        self.fullid = None
        self.livemountip = None
        self.livemounthost = None
        self.schname = None
        if 'backup' in action:
            self.status = 1
        else:
            self.status = 0
