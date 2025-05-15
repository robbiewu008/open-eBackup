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
class CopyTopic:
    COPY_RETENTION_TOPIC = "copy.retention"
    COPY_SAVE_TOPIC = "copy.save.request"
    COPY_CHECK_TOPIC = "copy.check.request"
    COPY_DELETE_REQUEST_TOPIC = "copy.delete.request"
    COPY_DELETED_TOPIC = "copy.delete.job.monitor.finished"
    PROTECTION_REMOVE_EVENT = "protection.remove.event"
    COPY_DELETE_INIT = "copy.delete.init"
    COPY_DELETE_LOCK = "copy.delete.locked"
    COPY_SAVE_EVENT = "copy.save.event"
    COPY_REPLICA_SUCCESS = "copy.replica.success"
