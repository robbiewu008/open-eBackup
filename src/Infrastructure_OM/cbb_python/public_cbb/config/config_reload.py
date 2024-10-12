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
#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import threading
import time
from pathlib import Path
from typing import Callable

from public_cbb.log.logger import get_logger
from public_cbb.security.anonym_utils.anonymity import Anonymity

log = get_logger()
TIME_INTERVAL = 30


class ConfigReload:
    def __init__(self, file: Path, reload_func: Callable):
        self.conf_file = Path(file)
        self.conf_mtime: float = None
        self.conf_loader = None
        self.reload_func = reload_func

    def run(self) -> None:
        self.conf_loader = threading.Thread(target=self.reload, name="config_reloader")
        self.conf_loader.start()
        log.info(f'ConfigReload thread is running...')

    def should_reload(self) -> bool:
        try:
            mtime = self.conf_file.stat().st_mtime
        except OSError:  # pragma: nocover
            return False

        if self.conf_mtime is None:
            self.conf_mtime = mtime
        elif mtime > self.conf_mtime:
            display_path = str(self.conf_file)
            try:
                display_path = str(self.conf_file.relative_to(Path.cwd()))
            except ValueError:
                pass
            log.info(f'ConfigReload detected file change in display_path:{display_path}. Reloading...')
            self.conf_mtime = mtime
            return True
        return False

    def reload(self) -> None:
        while True:
            try:
                if self.should_reload():
                    self.reload_func()
            except Exception as e:
                log.error(f'Catch except:{Anonymity.process(str(e))} while reloading')
            finally:
                time.sleep(TIME_INTERVAL)
