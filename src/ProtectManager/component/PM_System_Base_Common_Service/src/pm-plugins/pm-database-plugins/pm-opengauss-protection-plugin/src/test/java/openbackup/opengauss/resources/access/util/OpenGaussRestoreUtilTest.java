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
package openbackup.opengauss.resources.access.util;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.opengauss.resources.access.provider.OpenGaussMockData;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;

import java.util.List;

/**
 * OpenGaussRestoreUtil测试类
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-20
 */
public class OpenGaussRestoreUtilTest {
    /**
     * 用例场景  检查恢复任务的环境信息是否存在
     * 前置条件：恢复任务环境信息正常
     * 检查点: 环境信息不存在，抛出异常
     */
    @Test
    public void should_throw_legoCheckedException_if_env_is_not_exists_when_check_environment_exist_and_build_nodes() {
        RestoreTask restoreTask = new RestoreTask();
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> OpenGaussRestoreUtil.checkEnvironmentExistAndBuildNodes(restoreTask.getTargetEnv()));
        Assert.assertEquals("targetEnv is empty", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.OBJ_NOT_EXIST, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景  环境扩展字段的node转化为环境的nodes
     * 前置条件：恢复任务环境信息正常
     * 检查点: 环境扩展字段的node转化为nodes是否正确
     */
    @Test
    public void check_environment_exist_and_build_nodes_success() {
        RestoreTask restoreTask = OpenGaussMockData.buildRestoreTaskEnvNodeInfo();
        OpenGaussRestoreUtil.checkEnvironmentExistAndBuildNodes(restoreTask.getTargetEnv());
        List<TaskEnvironment> nodes = restoreTask.getTargetEnv().getNodes();
        Assert.assertEquals(1, nodes.size());
        Assert.assertEquals("node_1", nodes.get(0).getName());
    }
}