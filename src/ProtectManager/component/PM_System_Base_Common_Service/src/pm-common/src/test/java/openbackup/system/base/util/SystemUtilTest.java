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
package openbackup.system.base.util;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.context.ApplicationContext;

import java.security.Permission;

import junit.framework.TestCase;
import openbackup.system.base.util.SystemUtil;

/**
 * SystemUtilTest
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-25
 */
public class SystemUtilTest extends TestCase {
    protected static class ExitException extends SecurityException {
        public final int status;
        public ExitException(int status) {
            super("exit exception");
            this.status = status;
        }
    }

    private static class NoExitSecurityManager extends SecurityManager {
        @Override
        public void checkPermission(Permission perm) {
        }

        @Override
        public void checkPermission(Permission perm, Object context) {
        }

        @Override
        public void checkExit(int status) {
            super.checkExit(status);
            throw new ExitException(status);
        }
    }

    @Before
    public void setUp() {
        System.setSecurityManager(new NoExitSecurityManager());
    }

    @After
    public void tearDown() {
        System.setSecurityManager(null);
    }

    /**
     * 用例场景：健康检查调用SystemUtil的exit
     * 前置条件：检查检查需要停止服务正常
     * 检查点：正常退出JVM
     */
    @Test
    public void testExit() {
        ApplicationContext applicationContext;
        try {
            applicationContext = PowerMockito.mock(ApplicationContext.class);
            SystemUtil.stopApplication(applicationContext);
        } catch (ExitException e) {
            Assert.assertEquals(1, e.status);
        }
    }
}
