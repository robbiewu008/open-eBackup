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
import uuid

from app.common import logger

log = logger.get_logger(__name__)


def assert_in_policy(name, values, policy, xinfo):
    if name not in policy:
        err = f"invalid policy: missing {name}, xinfo={xinfo}"
        log.error(err)
        raise KeyError(err)
    if policy[f"{name}"] not in values:
        err = f"invalid policy: name={name}, xinfo={xinfo}"
        log.error(err)
        raise KeyError(err)


def get_interval_minutes(interval_s):
    """
    Input: string with the regex format: [:digit:]+[smhdw]
    e.g. 12h, 3d, 6w meaning 12 hours, 3 days, 6 weeks.
    output: input value in hours as integer
    Note: small durations are rounded up to 1 hour
    """
    duration = int(interval_s[:-1])
    d_type = interval_s[-1:]
    if d_type == "m":
        return duration
    if d_type == "h":
        return duration * 60
    if d_type == "d":
        return duration * 60 * 24
    if d_type == "w":
        return duration * 60 * 24 * 7
    # seconds rounded up to 1 min
    return 1


def get_backup_interval(policy):
    values = ["full", "incremental"]
    assert_in_policy("backup_mode", values, policy, xinfo="get_backup_interval")

    interval = get_interval_minutes(policy["interval"])
    interval = str(interval) + "m"

    log.debug(f"backup interval={interval}")
    return interval


def suuid():
    return str(uuid.uuid4())
