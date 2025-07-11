/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.common.system.event;

import org.springframework.context.ApplicationEvent;

import java.util.Map;

/**
 * SystemConfigChangeEvent
 *
 */
public class SystemConfigChangeEvent extends ApplicationEvent {
    private final Map<String, String> changedConfigMap;

    public SystemConfigChangeEvent(Object source, Map<String, String> changedConfigMap) {
        super(source);
        this.changedConfigMap = changedConfigMap;
    }

    public Map<String, String> getChangedConfigMap() {
        return changedConfigMap;
    }
}
