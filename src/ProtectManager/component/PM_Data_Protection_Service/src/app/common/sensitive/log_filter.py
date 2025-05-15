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
import re
from typing import List


from app.conf.sensitive_rule_config import config


class LogFilter(logging.Filter):
    fuzz_keys: List[str] = config.get_rule_keys("fuzz")
    word_keys: List[str] = config.get_rule_keys("word")

    def filter(self, record: logging.LogRecord):
        msg = record.msg
        if LogFilter.fuzz_keys and any(temp in msg.lower() for temp in LogFilter.fuzz_keys):
            return False
        if LogFilter.word_keys and any(self._match_word(temp, msg) for temp in LogFilter.word_keys):
            return False
        return True

    def _match_word(self, key: str, msg: str):
        return re.search(rf"\b{key}\b", msg, re.M | re.I)