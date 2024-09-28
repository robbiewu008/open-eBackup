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
package openbackup.database.base.plugin.service.impl;

import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.service.impl.DatabaseRestoreServiceImpl;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;

/**
 * {@link DatabaseRestoreServiceImpl 测试类}
 *
 */
public class DatabaseRestoreServiceImplTest {
    private DatabaseRestoreServiceImpl databaseRestoreService = new DatabaseRestoreServiceImpl();

    /**
     * 用例场景：校验恢复的源端和目标端部署操作系统是否一致
     * 前置条件：不一致
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_source_and_target_os_inconsistent_when_check_os() {
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> databaseRestoreService.checkDeployOperatingSystem("red hat", "suse"));
        Assert.assertEquals(DatabaseErrorCode.RESTORE_OS_INCONSISTENT, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：校验恢复的源端和目标端部署操作系统是否一致
     * 前置条件：一致
     * 检查点：无异常抛出
     */
    @Test
    public void execute_check_os_success() {
        databaseRestoreService.checkDeployOperatingSystem("suse", "suse");
        Assert.assertThrows(LegoCheckedException.class, ()->databaseRestoreService.checkDeployOperatingSystem("suse", "s"));
    }

    /**
     * 用例场景：校验恢复的源端和目标端资源类型是否一致
     * 前置条件：不一致
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_source_and_target_resource_type_inconsistent_when_check_type() {
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> databaseRestoreService.checkResourceSubType("DB2-database", "DB2-tablespace"));
        Assert.assertEquals(DatabaseErrorCode.RESTORE_RESOURCE_TYPE_INCONSISTENT, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：校验恢复的源端和目标端资源类型是否一致
     * 前置条件：一致
     * 检查点：无异常抛出
     */
    @Test
    public void execute_check_resource_type_success() {
        databaseRestoreService.checkResourceSubType("DB2-database", "DB2-database");
        Assert.assertThrows(LegoCheckedException.class, ()->databaseRestoreService.checkResourceSubType("DB2-database", "D"));
    }

    /**
     * 用例场景：校验恢复的源端和目标端集群类型是否一致
     * 前置条件：不一致
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_source_and_target_cluster_type_inconsistent_when_check_type() {
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> databaseRestoreService.checkClusterType("dpf", "powerHA"));
        Assert.assertEquals(DatabaseErrorCode.RESTORE_TARGET_RESOURCE_TYPE_INCONSISTENT, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：校验恢复的源端和目标端集群类型是否一致
     * 前置条件：一致
     * 检查点：无异常抛出
     */
    @Test
    public void execute_check_cluster_type_success() {
        databaseRestoreService.checkClusterType("dpf", "dpf");
        Assert.assertThrows(LegoCheckedException.class, ()->databaseRestoreService.checkClusterType("dpf", "d"));
    }

    /**
     * 用例场景：校验恢复的源端和目标端资源版本是否一致
     * 前置条件：不一致
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_source_and_target_version_inconsistent_when_check_version() {
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> databaseRestoreService.checkVersion("10.5", "11.1"));
        Assert.assertEquals(DatabaseErrorCode.RESTORE_RESOURCE_VERSION_INCONSISTENT, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：校验恢复的源端和目标端资源版本是否一致
     * 前置条件：一致
     * 检查点：无异常抛出
     */
    @Test
    public void execute_check_version_success() {
        databaseRestoreService.checkVersion("10.5", "10.5");
        Assert.assertThrows(LegoCheckedException.class, () -> databaseRestoreService.checkVersion("10.5", "11.1"));
    }
}
