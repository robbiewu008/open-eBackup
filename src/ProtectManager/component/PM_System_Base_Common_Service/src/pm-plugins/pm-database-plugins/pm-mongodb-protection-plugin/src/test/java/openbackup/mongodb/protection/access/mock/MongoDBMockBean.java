package openbackup.mongodb.protection.access.mock;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.mongodb.protection.access.constants.MongoDBConstants;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * MongoDB 工具构造类
 *
 * @author swx1010572
 * @version [DataBackup 1.5.0]
 * @since 2023-04-27
 */
public class MongoDBMockBean {
    public ProtectedEnvironment getMongoDBProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setName("username");
        protectedEnvironment.setExtendInfo(new HashMap<>());
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.CHILDREN, new ArrayList<>());
        protectedEnvironment.setDependencies(dependencies);
        protectedEnvironment.setOsType("linux");
        protectedEnvironment.setLinkStatus(String.valueOf(LinkStatusEnum.ONLINE.getStatus()));
        return protectedEnvironment;
    }

    public AppEnvResponse getMongoDBAppEnvResponse() {
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        Map<String, String> appEnvExtendInfo = new HashMap<>();
        appEnvExtendInfo.put(MongoDBConstants.DATA_PATH,"/home/mongodb/route/data");
        appEnvExtendInfo.put(MongoDBConstants.SHARD_CLUSTER_TYPE,"datapath");
        appEnvExtendInfo.put(MongoDBConstants.ARGV,"datapath");
        appEnvExtendInfo.put(MongoDBConstants.CONFIG_PATH,"/home/mongodb/route/mongos.conf");
        appEnvExtendInfo.put(MongoDBConstants.LOCAL_HOST,"8.40.96.214:28017");
        appEnvExtendInfo.put(MongoDBConstants.AGENT_NODES,"8.40.96.214:28017");
        appEnvExtendInfo.put(MongoDBConstants.CLUSTE_TYPE,"0");
        appEnvExtendInfo.put(DatabaseConstants.VERSION,"4.2");
        appEnvResponse.setExtendInfo(appEnvExtendInfo);
        return appEnvResponse;
    }
}
