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
import logging
import os
from contextlib import suppress
from dataclasses import dataclass
from functools import partial
from app.common import logger


__all__ = ["config", "reload"]


log = logger.get_logger(__name__)


@dataclass
class Config:
    log_level: int = logging.DEBUG
    user_name: str = "example user"


config = Config()


def reload(sig, frame):
    global config
    get = partial(os.environ.get, default="")

    config.log_level = logging.DEBUG
    log_level = get("LOG_LEVEL")
    with suppress(ValueError):
        config.log_level = int(log_level)
    with suppress(AttributeError):
        config.log_level = getattr(logger, log_level)

    config.user_name = get("USERNAME")

    log.info("reload config")


reload(None, None)
