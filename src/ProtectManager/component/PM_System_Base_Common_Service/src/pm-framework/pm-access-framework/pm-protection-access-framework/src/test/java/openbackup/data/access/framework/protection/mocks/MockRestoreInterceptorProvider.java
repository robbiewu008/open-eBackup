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
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;

import org.springframework.beans.BeanUtils;

/**
 * 恢复应用插件的模拟类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/15
 **/
public class MockRestoreInterceptorProvider implements RestoreInterceptorProvider {
    @Override
    public boolean applicable(String object) {
        return true;
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        TaskResource mockResource = new TaskResource();
        BeanUtils.copyProperties(ProtectedResourceMocker.mockTaskResource(), mockResource);
        task.setTargetObject(mockResource);
        return task;
    }
}
