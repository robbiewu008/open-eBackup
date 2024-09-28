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
package openbackup.mongodb.protection.access.util;

import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.mongodb.protection.access.bo.MongoClusterNodesExtendInfo;
import openbackup.mongodb.protection.access.constants.MongoDBConstants;

import org.junit.Assert;
import org.junit.Test;

import java.util.HashMap;

/**
 * mongodb 工具类LLT
 *
 */
public class MongoDBConstructionUtilsTest {

    /**
     * 用例场景：构造mongo集群的工具类
     * 前置条件：无
     * 检查点：正常通过和正常报错
     */
    @Test
    public void build_mongo_cluster_nodes_extend_info_success() {
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setExtendInfo(new HashMap<>());
        nodeInfo.getExtendInfo().put(MongoDBConstants.PRIORITY, "1");
        MongoClusterNodesExtendInfo mongoClusterNodesExtendInfo = MongoDBConstructionUtils.buildMongoClusterNodesExtendInfo(new HashMap<>(), nodeInfo);
        Assert.assertEquals(mongoClusterNodesExtendInfo.getPriority(), "1");
    }
}
