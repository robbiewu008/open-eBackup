package openbackup.mysql.resources.access.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DbClusterProvider;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.common.MysqlErrorCode;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;

import com.google.common.collect.Maps;

import lombok.extern.slf4j.Slf4j;
import reactor.util.function.Tuple2;
import reactor.util.function.Tuples;

import org.apache.commons.collections4.CollectionUtils;
import org.apache.commons.collections4.SetUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * eappMySQL集群实例接入
 *
 * @author fWX1071802
 * @since 2023/6/12
 */
@Component
@Slf4j
public class EAppMysqlClusterInstanceProvider implements DbClusterProvider {
    private AgentUnifiedService agentUnifiedService;

    private MysqlBaseService mysqlBaseService;

    @Autowired
    public void setAgentUnifiedService(AgentUnifiedService agentUnifiedService) {
        this.agentUnifiedService = agentUnifiedService;
    }

    @Autowired
    public void setMysqlBaseService(MysqlBaseService mysqlBaseService) {
        this.mysqlBaseService = mysqlBaseService;
    }

    @Override
    public boolean checkIsCluster(ProtectedResource protectedResource) {
        Map<String, List<ProtectedResource>> dependencies = protectedResource.getDependencies();
        if (dependencies == null) {
            // 修改操作时，子实例不允许为空
            if (protectedResource.getProtectionStatus() == null) {
                throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "subInstance can not be empty");
            }
            return true;
        }
        List<ProtectedResource> instanceList = dependencies.get(DatabaseConstants.CHILDREN);
        if (CollectionUtils.isEmpty(instanceList)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "subInstance can not be empty");
        }
        Map<List<String>, String> instanceIpMap = Maps.newHashMapWithExpectedSize(instanceList.size());
        Map<String, List<String>> instanceMasterMap = Maps.newHashMapWithExpectedSize(instanceList.size());
        instanceList.forEach(instance -> {
            Tuple2<List<String>, List<String>> masterMap = queryAgentInfo(instance);
            instanceIpMap.put(masterMap.getT1(), instance.getName());
            instanceMasterMap.put(instance.getName(), masterMap.getT2());
        });
        // 将master的ip转换为对应的agent节点id
        Map<String, List<String>> instanceMasterInfo = instanceMasterMap.entrySet()
            .stream()
            .collect(Collectors.toMap(Map.Entry::getKey, entry -> {
                List<String> masterIpList = entry.getValue();
                // 将master列表转换为对应的节点列表
                return masterIpList.stream()
                    .map(masterIp -> instanceIpMap.entrySet()
                        .stream()
                        .filter(ipInfo -> ipInfo.getKey().contains(masterIp))
                        .map(Map.Entry::getValue)
                        .findFirst()
                        .orElse(null))
                    .collect(Collectors.toList());
            }));
        if (!isCorrectMaster(instanceMasterInfo)) {
            throw new LegoCheckedException(MysqlErrorCode.CHECK_CLUSTER_FAILED,
                "The number of cluster nodes does not match");
        }
        return true;
    }

    private boolean isCorrectMaster(Map<String, List<String>> instanceMasterInfo) {
        Set<String> instanceKeys = instanceMasterInfo.keySet();
        return instanceMasterInfo.entrySet().stream().allMatch(entry -> {
            // 正确的master列表为排除当前节点后的所有节点
            Set<String> correctMasterSet = instanceKeys.stream()
                .filter(key -> !Objects.equals(key, entry.getKey()))
                .collect(Collectors.toSet());
            return SetUtils.isEqualSet(correctMasterSet, entry.getValue());
        });
    }

    @Override
    public boolean applicable(String clusterType) {
        return MysqlConstants.MYSQL_EAPP.equals(clusterType);
    }

    private Tuple2<List<String>, List<String>> queryAgentInfo(ProtectedResource resource) {
        // 修改时，密码可能为空，需重新赋值密码
        if (VerifyUtil.isEmpty(resource.getAuth().getAuthPwd())) {
            ProtectedResource oldInstance = mysqlBaseService.getResource(resource.getUuid());
            resource.getAuth().setAuthPwd(oldInstance.getAuth().getAuthPwd());
        }
        String envId = resource.getExtendInfo().get(DatabaseConstants.HOST_ID);
        ProtectedEnvironment environment = mysqlBaseService.getEnvironmentById(envId);
        resource.setPath(environment.getEndpoint());
        AppEnvResponse appEnvResponse = agentUnifiedService.getClusterInfoNoRetry(resource, environment);
        if (appEnvResponse == null || appEnvResponse.getExtendInfo() == null) {
            throw new DataProtectionAccessException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED, new String[] {},
                "Get mysql cluster error.");
        }
        Map<String, String> envExtendInfo = appEnvResponse.getExtendInfo();
        String errorCode = envExtendInfo.get(MysqlConstants.ERROR_CODE);
        if (StringUtils.isNotBlank(errorCode) && !(DatabaseConstants.SUCCESS_CODE + "").equals(errorCode)) {
            throw new DataProtectionAccessException(Integer.parseInt(errorCode), new String[] {},
                "Get mysql cluster error.");
        }
        String masterIp = envExtendInfo.getOrDefault(MysqlConstants.MASTER_LIST, "");
        List<String> masterIpList = Arrays.asList(masterIp.split(","));
        String localIpStr = envExtendInfo.getOrDefault(MysqlConstants.CURRENT_IP_LIST, "");
        List<String> localIpList = Arrays.asList(localIpStr.split(","));
        return Tuples.of(localIpList, masterIpList);
    }
}
