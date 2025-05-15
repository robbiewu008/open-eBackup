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
import os
import logging
from logging import DEBUG, INFO, WARN, ERROR, FATAL
from contextlib import suppress
from pythonjsonlogger import jsonlogger


_FORMAT_STRING = "%(asctime)s %(name)s:%(threadName)s:%(levelname)s, %(filename)s:%(lineno)s;%(funcName)s; %(message)s"
_handler = logging.StreamHandler()
_formatter = jsonlogger.JsonFormatter(_FORMAT_STRING)
_handler.setFormatter(_formatter)
set_level = _handler.setLevel


def get_logger(name):
    logger = logging.getLogger(name)
    if not logger.handlers:
        log_level = os.getenv('LOGLEVEL', default='INFO')
        with suppress(AttributeError):
            log_level = getattr(logging, log_level)
        logger.setLevel(log_level)
        logger.addHandler(_handler)
        logger.propagate = False
    return logger
