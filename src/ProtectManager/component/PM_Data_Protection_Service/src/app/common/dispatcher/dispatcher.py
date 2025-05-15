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
class StateDispatcher(object):

    def __init__(self, state_attr='state'):
        self.registry = {}
        self._state_attr = state_attr

    def __get__(self, instance, owner):
        if instance is None:
            return self

        method = self.registry[getattr(instance, self._state_attr)]
        return method.__get__(instance, owner)

    def register(self, *state):
        def decorator(method):
            for item in state:
                self.registry[item] = method
            return method

        return decorator
