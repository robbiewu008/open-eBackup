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
package openbackup.data.access.client.sdk.api.framework.dme;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.beans.IntrospectionException;
import java.lang.reflect.InvocationTargetException;

/**
 * DmeCopyInfo LLT
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({DmeCopyInfo.class})
public class DmeCopyInfoTest extends DmeTest {

    @Before
    public void init() throws IntrospectionException, InvocationTargetException, InstantiationException, IllegalAccessException {
        this.getAndSetTest();
    }

    /**
     * 用例场景：根据dme副本中的校验文件获取副本校验状态
     * 前置条件：
     * 检查点：获取的值不为空
     */
    @Test
    public void should_copy_verify_status_enum() {
        DmeCopyInfo dmeCopyInfo = new DmeCopyInfo();
        dmeCopyInfo.equals("test");
        dmeCopyInfo.hashCode();
        dmeCopyInfo.toString();
        CopyVerifyStatusEnum copyVerifyStatus = dmeCopyInfo.getCopyVerifyStatus();
        Assert.assertNotNull(copyVerifyStatus);
    }

    @Override
    protected DmeCopyInfo getT() {
        return new DmeCopyInfo();
    }
}
