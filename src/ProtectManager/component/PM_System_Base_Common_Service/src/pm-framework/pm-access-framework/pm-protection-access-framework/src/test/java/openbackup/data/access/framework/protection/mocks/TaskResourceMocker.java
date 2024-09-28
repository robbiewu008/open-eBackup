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
package openbackup.data.access.framework.protection.mocks;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * 任务资源的mock工具类
 *
 **/
public class TaskResourceMocker {

    public static TaskResource mockFullInfo(){
        TaskResource resource = new TaskResource();
        resource.setUuid(UUID.randomUUID().toString());
        resource.setName("test_resource");
        resource.setType("Database");
        resource.setSubType("Oracle");
        resource.setParentName("parent_test_resource");
        resource.setParentUuid(UUID.randomUUID().toString());
        resource.setRootUuid(UUID.randomUUID().toString());
        resource.setPath("/parent/children");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("key1", "value1");
        extendInfo.put("key2", "value2");
        resource.setExtendInfo(extendInfo);
        return resource;
    }
}
