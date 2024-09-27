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
package openbackup.database.base.plugin.interceptor;

import openbackup.data.access.framework.copy.mng.provider.BaseCopyDeleteInterceptor;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * 测试副本删除拦截器类
 *
 * @author h30027154
 * @since 2022-06-16
 */
@Component
public class TestBaseCopyDeleteInterceptor extends BaseCopyDeleteInterceptor {
    @Override
    public boolean applicable(String object) {
        return true;
    }

    @Override
    public List<String> getAssociatedCopy(String copyId) {
        return Arrays.asList("1", "2");
    }

    @Override
    protected boolean shouldSupplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        return true;
    }
}
