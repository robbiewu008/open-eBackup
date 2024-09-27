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
import re

regex = re.compile(r'\<((IMAGE|SERVICE|USER)_?NAME|AUTHOR(MAIL)?|DESCRIPTION)\>')
fregex = re.compile(r'\.(ya?ml|java|sh)$|^\.?docker.+$', re.IGNORECASE)

exclude_dirs = set(['.git', '__pycache__'])

found = set()
print('searching for service boilerplate tags...')

for root, dirs, files in os.walk('./', topdown=True):
    dirs[:] = [d for d in dirs if d not in exclude_dirs]
    for fname in files:
        if not fregex.search(fname) or re.search(r'^check.*boil', fname):
            continue
        path = '{}/{}'.format(root, fname)
        with open(path) as f:
            nof_line = 0
            for line in f:
                nof_line = nof_line + 1
                result = regex.search(line)
                if result:
                    found.add(path)
                    print('{}:{} {}'.format(path, nof_line, result.group(1)))

print('total boilerplate tags: {}'.format(len(found)))
exit(len(found))
