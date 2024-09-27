package openbackup.database.base.plugin.service.impl;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import org.springframework.stereotype.Service;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 数据库的实例保护服务
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-21
 */
@Service
public class InstanceProtectionServiceImpl implements InstanceProtectionService {
    @Override
    public List<TaskEnvironment> extractEnvNodesBySingleInstance(ProtectedResource resource) {
        return resource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .map(childNode -> buildTaskEnvironment(childNode, resource))
            .collect(Collectors.toList());
    }

    private TaskEnvironment buildTaskEnvironment(ProtectedEnvironment environment, ProtectedResource resource) {
        Map<String, String> extendInfo = Optional.of(environment.getExtendInfo()).orElseGet(HashMap::new);
        if (!VerifyUtil.isEmpty(resource.getExtendInfo())) {
            extendInfo.putAll(resource.getExtendInfo());
        }
        environment.setExtendInfo(extendInfo);
        environment.setAuth(resource.getAuth());
        return BeanTools.copy(environment, TaskEnvironment::new);
    }

    @Override
    public List<TaskEnvironment> extractEnvNodesByClusterInstance(ProtectedResource resource) {
        return resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .map(this::extractEnvNodesBySingleInstance)
            .flatMap(List::stream)
            .collect(Collectors.toList());
    }
}
