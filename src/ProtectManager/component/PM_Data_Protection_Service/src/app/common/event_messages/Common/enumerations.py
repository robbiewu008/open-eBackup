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
from enum import Enum


class ProtectedObjectType(str, Enum):
    db = "db"
    vm = "vm"
    volume = "volume"
    fileset = "fileset"
    snap = "snapshot"   # Used by snapshot delete


class EnvType(str, Enum):
    vmware = "vmware"
    fusionsphere = "fusionsphere"
    oracle = "oracle"
    exchange = "exchange"
    linux = "linux"
    windows = "windows"
    empty = ""


class SchedulingType(str, Enum):
    immediate = "immediate"
    delayed = "delayed"
    schedule = "schedule"
    interval = "interval"


class SchedulerAction(str, Enum):
    backup = "backup"
    restore = "restore"
    delete = "delete"
    backup_host = "backup_host"
    restore_files = "restore_files"
    abort = "abort"
    livemount_vm_add = "livemount_add"
    livemount_vm_remove = "livemount_remove"
    livemount_vm_get_list = "livemount_get_list"
    livemount_vm_edit = "livemount_edit"
    livemount_vm_update = "livemount_update"
