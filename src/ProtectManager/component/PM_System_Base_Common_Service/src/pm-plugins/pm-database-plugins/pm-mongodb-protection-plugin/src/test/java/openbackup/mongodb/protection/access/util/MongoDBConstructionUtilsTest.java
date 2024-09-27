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
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
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
