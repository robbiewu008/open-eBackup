/*
 *
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

package openbackup.database.base.plugin.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceReq;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.AppConf;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.database.base.plugin.common.GeneralDbErrorCode;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.util.GeneralDbUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.StreamUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * GeneralDb的ResourceProvider实现
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-22
 */
@Component
@Slf4j
public class GeneralDbResourceProvider implements ResourceProvider {
    private static final String UPDATE_DELETE_CONNECTION_PREFIX = "-";

    private static final String UPDATE_DELETE_RESOURCE_PREFIX = "#";

    private static final String EXTEND_COMMON_SEPARATOR = ",";

    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    public GeneralDbResourceProvider(ResourceService resourceService, AgentUnifiedService agentUnifiedService) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.equals(object.getSubType(), ResourceSubTypeEnum.GENERAL_DB.getType());
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        // 检查数据库是否已在 SAP HANA 应用中注册
        checkDbIsRegisteredInSapHana(resource);
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
    }

    @Override
    public void check(ProtectedResource resource) {
        resource.setUuid(null);
        if (resource instanceof ProtectedEnvironment) {
            resource.setRootUuid(null);
        }
        checkEssentialRegisterParam(resource);

        List<ProtectedEnvironment> hosts = findHost(resource);
        handleHostInfo(resource, hosts);
        checkName(resource, hosts);
        Map<String, Object> confMap = doQueryAppConf(resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY),
            hosts, false);
        checkScript(resource, confMap);

        findResource(resource, null, hosts);

        checkAfterSupport(resource, confMap);
    }

    @Override
    public void updateCheck(ProtectedResource resource) {
        // 需要删除的dependency先排除在外
        Map<String, List<ProtectedResource>> toDeleteDependencies = excludeToBeDeleteDependency(resource);

        List<ProtectedEnvironment> hosts = findHost(resource);
        handleHostInfo(resource, hosts);
        checkName(resource, hosts);

        // 原有数据库资源
        ProtectedResource resourceInDb = resourceService.getResourceById(resource.getUuid())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "resource does not exist"));
        Map<String, Object> confMap = doQueryAppConf(resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY),
            hosts, false);
        // 检查脚本变化
        checkScriptChange(resource, resourceInDb, confMap);

        findResource(resource, resourceInDb, hosts);
        updateCheckAfterSupport(resource, confMap);
        // 恢复要删除的dependency
        Optional.ofNullable(resource.getDependencies()).ifPresent(e -> e.putAll(toDeleteDependencies));
    }

    private Map<String, List<ProtectedResource>> excludeToBeDeleteDependency(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = resource.getDependencies();
        Map<String, List<ProtectedResource>> toDeleteDependencies = new HashMap<>();
        if (dependencies == null) {
            return toDeleteDependencies;
        }
        for (Map.Entry<String, List<ProtectedResource>> entry : dependencies.entrySet()) {
            if (entry.getKey().startsWith(UPDATE_DELETE_CONNECTION_PREFIX) || entry.getKey()
                .startsWith(UPDATE_DELETE_RESOURCE_PREFIX)) {
                toDeleteDependencies.put(entry.getKey(), entry.getValue());
                dependencies.remove(entry.getKey());
            }
        }
        return toDeleteDependencies;
    }

    private void findResource(ProtectedResource resource, ProtectedResource resourceInDb,
        List<ProtectedEnvironment> hosts) {
        ListResourceReq listResourceReq = new ListResourceReq();
        desensitize(resourceInDb);
        listResourceReq.setAppEnv(GeneralDbUtil.resourceToAppEnv(resource, hosts, resourceInDb));
        Application application = new Application();
        application.setSubType(resource.getSubType());
        application.setExtendInfo(new HashMap<>());
        application.getExtendInfo()
            .put(GeneralDbConstant.EXTEND_SCRIPT_KEY, resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY));
        listResourceReq.setApplication(application);
        boolean shouldNextSupport = false;
        for (ProtectedEnvironment host : hosts) {
            AgentDetailDto agentDetailDto = agentUnifiedService.getDetail(resource.getSubType(), host.getEndpoint(),
                host.getPort(), listResourceReq);
            if (Optional.ofNullable(agentDetailDto)
                .map(AgentDetailDto::getResourceList)
                .orElse(new ArrayList<>())
                .isEmpty()) {
                throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "agent return's resource is empty");
            }
            GeneralDbUtil.appResourceToProtectedResource(resource, hosts, agentDetailDto.getResourceList().get(0));
            shouldNextSupport = Optional.ofNullable(
                resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_SHOULD_NEXT_SUPPORT_KEY))
                .map(Boolean::valueOf)
                .orElse(false);
            // 如果shouldNextSupport返回true, 则表示用下一个节点调用support
            if (shouldNextSupport) {
                continue;
            }
            break;
        }
        if (shouldNextSupport) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "shouldNextSupport is true");
        }
    }

    private void desensitize(ProtectedResource resource) {
        Optional.ofNullable(resource).ifPresent(resourceService::desensitize);
    }

    private void checkEssentialRegisterParam(ProtectedResource resource) {
        requiresNotEmpty(resource.getName(), "name is empty.");
        requiresNotEmpty(resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY), "script is empty.");
        requiresNotEmpty(resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_FIRST_CLASSIFICATION_KEY),
            "first classification is empty");
        String customParams = Optional.ofNullable(resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_CUSTOM_PARAM))
            .orElse("");
        requiresEqual(customParams.length() <= GeneralDbConstant.EXTEND_CUSTOM_PARAM_LENGTH, true,
            "custom params length can not exceed " + GeneralDbConstant.EXTEND_CUSTOM_PARAM_LENGTH);
        // 现在只支持数据库
        requiresEqual(resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_FIRST_CLASSIFICATION_KEY),
            GeneralDbConstant.GENERAL_DB_DATABASE, "only support database now.");

        // 数据库情形
        if (Objects.equals(resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_FIRST_CLASSIFICATION_KEY),
            GeneralDbConstant.GENERAL_DB_DATABASE)) {
            requiresNotEmpty(resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_DEPLOY_TYPE),
                "deploy type is empty.");
            // 判断部署类型与主机数量关系
            List<ProtectedResource> hosts = GeneralDbUtil.getHostsFromDependency(resource);
            if (Objects.equals(resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_DEPLOY_TYPE),
                DatabaseDeployTypeEnum.SINGLE.getType())) {
                requiresEqual(hosts.size(), 1, "host number is wrong.");
            } else {
                requiresEqual(hosts.size() > 1, true, "host number is wrong.");
            }
        }
    }

    private void requiresNotEmpty(String obj, String message) {
        if (VerifyUtil.isEmpty(obj)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, message == null ? "param is invalid." : message);
        }
    }

    private void requiresEqual(Object obj1, Object obj2, String message) {
        if (!Objects.equals(obj1, obj2)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, message == null ? "param is invalid." : message);
        }
    }

    @Override
    public Map<String, Object> queryAppConf(String script, String[] hostUuids) {
        if (VerifyUtil.isEmpty(hostUuids)) {
            return new HashMap<>();
        }
        List<ProtectedEnvironment> environments = queryHostWithId(Arrays.asList(hostUuids));
        return doQueryAppConf(script, environments, true);
    }

    private void checkScript(ProtectedResource resource, Map<String, Object> confMap) {
        String script = resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY);
        if (!confMap.containsKey(script)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "the param of script errors.");
        }
    }

    private void checkScriptChange(ProtectedResource resource, ProtectedResource resourceInDb,
        Map<String, Object> confMap) {
        String script = resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY);
        String scriptInDb = resourceInDb.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY);
        if (VerifyUtil.isEmpty(script)) {
            script = scriptInDb;
        }
        if (!Objects.equals(script, scriptInDb) || !confMap.containsKey(script)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "the param of script errors.");
        }
    }

    private void checkAfterSupport(ProtectedResource resource, Map<String, Object> confMap) {
        // 检查id这些
        checkUuid(resource);

        afterSupport(resource, confMap);

        fillResourcePath(resource);

        if (resource instanceof ProtectedEnvironment) {
            ProtectedEnvironment environment = (ProtectedEnvironment) resource;
            if (environment.getEndpoint() == null) {
                environment.setEndpoint("");
            }
            if (environment.getPath() == null) {
                environment.setPath("");
            }
        }
    }

    private void updateCheckAfterSupport(ProtectedResource resource, Map<String, Object> confMap) {
        afterSupport(resource, confMap);
    }

    private void afterSupport(ProtectedResource resource, Map<String, Object> confMap) {
        String script = resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY);
        String scriptConfStr = JsonUtil.json(confMap.get(script));
        // 设置conf
        resource.setExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_CONF, scriptConfStr);
        AppConf appConf = GeneralDbUtil.getAppConf(scriptConfStr).orElse(new AppConf());

        // 设置databaseType展示名
        resource.setExtendInfoByKey(GeneralDbConstant.DATABASE_TYPE_DISPLAY, appConf.getDatabaseType());

        // 设置状态
        if (resource instanceof ProtectedEnvironment) {
            ((ProtectedEnvironment) resource).setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        }
    }

    private void fillResourcePath(ProtectedResource resource) {
        List<String> resourceIds = resource.getDependencies().get(GeneralDbConstant.DEPENDENCY_HOST_KEY).stream()
                .map(ResourceBase::getUuid)
                .collect(Collectors.toList());
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("uuid", resourceIds);
        List<String> endpoints = resourceService.basicQuery(false, 0, resourceIds.size(), conditions)
                .getRecords()
                .stream()
                .map(ProtectedResource::getEndpoint)
                .collect(Collectors.toList());
        if (!VerifyUtil.isEmpty(endpoints)) {
            resource.setPath(String.join(DatabaseConstants.SPLIT_CHAR, endpoints));
        }
    }

    private void checkUuid(ProtectedResource resource) {
        if (VerifyUtil.isEmpty(resource.getUuid())) {
            resource.setUuid(UUID.randomUUID().toString());
        }
        if (VerifyUtil.isEmpty(resource.getRootUuid())) {
            resource.setRootUuid(resource.getUuid());
        }
    }

    /**
     * 查询所有主机的配置项，返回他们的并集结果。即这些主机都要支持某种应用。
     *
     * @param script 脚本
     * @param environments 环境信息
     * @param isSilent 查询不到时是否抛出异常
     * @return 配置信息
     */
    private Map<String, Object> doQueryAppConf(String script, List<ProtectedEnvironment> environments,
        boolean isSilent) {
        HashMap<String, Object> res = new HashMap<>();
        List<Map<String, Object>> confMaps = new ArrayList<>();
        for (ProtectedEnvironment env : environments) {
            Map<String, Object> appConf = queryConf(env, script);
            log.info("ip is {}, support script is {}", env.getEndpoint(), appConf.keySet());
            if (VerifyUtil.isEmpty(appConf)) {
                return new HashMap<>();
            }
            confMaps.add(appConf);
        }
        Set<String> unionKeys = getUnionKey(confMaps);
        for (String unionKey : unionKeys) {
            Optional<Map<String, Object>> optionalMap = confMaps.stream()
                .filter(e -> e.containsKey(unionKey))
                .findFirst();
            optionalMap.ifPresent(e -> res.put(unionKey, e.get(unionKey)));
        }
        if (!isSilent && VerifyUtil.isEmpty(res)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Can not find conf.");
        }
        return res;
    }

    private static Set<String> getUnionKey(List<Map<String, Object>> confMaps) {
        Set<String> res = new HashSet<>();
        confMaps.forEach(e -> res.addAll(e.keySet()));
        for (Map<String, Object> confMap : confMaps) {
            res.retainAll(confMap.keySet());
        }
        return res;
    }

    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = ResourceProvider.super.getResourceFeature();
        resourceFeature.setShouldCheckEnvironmentNameDuplicate(false);
        resourceFeature.setShouldCheckResourceNameDuplicate(false);
        return resourceFeature;
    }

    private Map<String, Object> queryConf(ProtectedEnvironment env, String script) {
        Map<String, Object> confMap = agentUnifiedService.queryAppConf(ResourceSubTypeEnum.GENERAL_DB.getType(), script,
            env.getEndpoint(), env.getPort());
        return Optional.ofNullable(confMap).orElse(new HashMap<>());
    }

    private List<ProtectedEnvironment> findHost(ProtectedResource resource) {
        List<ProtectedResource> hosts = GeneralDbUtil.getHostsByDependencyKey(resource);
        List<String> hostUuids = hosts.stream()
            .map(ResourceBase::getUuid)
            .filter(Objects::nonNull)
            .collect(Collectors.toList());
        return queryHostWithId(hostUuids);
    }

    private List<ProtectedEnvironment> queryHostWithId(List<String> hostUuids) {
        if (VerifyUtil.isEmpty(hostUuids)) {
            return Collections.emptyList();
        }

        ResourceQueryParams params = new ResourceQueryParams();
        params.setSize(hostUuids.size());
        params.setShouldLoadEnvironment(false);
        params.setConditions(Collections.singletonMap("uuid", hostUuids));
        List<ProtectedResource> records = resourceService.query(params).getRecords();
        return records.stream()
            .filter(e -> e instanceof ProtectedEnvironment)
            .map(e -> (ProtectedEnvironment) e)
            .collect(Collectors.toList());
    }

    private void checkName(ProtectedResource resource, List<ProtectedEnvironment> hosts) {
        if (resource.getName() == null) {
            return;
        }
        String script = getScriptFromResource(resource);
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(GeneralDbConstant.EXTEND_SCRIPT_KEY, script);

        Set<String> findResourceUuids = queryAllResources(conditions)
            .stream()
            .filter(e -> resource.getName().equalsIgnoreCase(e.getName()))
            .filter(e -> isResourceInDbContainsHost(e, hosts))
            .map(ResourceBase::getUuid)
            .collect(Collectors.toSet());
        // 修改场景需要移除自身
        if (Objects.nonNull(resource.getUuid())) {
            findResourceUuids.remove(resource.getUuid());
        }
        if (findResourceUuids.size() > 0) {
            throw new LegoCheckedException(GeneralDbErrorCode.GENERAL_DB_DUPLICATE_NAME,
                new String[] {resource.getName()}, "Duplicate resource name exists");
        }
    }

    private void handleHostInfo(ProtectedResource resource, List<ProtectedEnvironment> hosts) {
        // 按uuid顺序检出 host ips 和 ids
        List<ProtectedEnvironment> sortedHost = hosts.stream()
            .sorted(Comparator.comparing(ProtectedResource::getUuid))
            .collect(Collectors.toList());
        String hostIps = sortedHost.stream()
            .map(ProtectedEnvironment::getEndpoint)
            .collect(Collectors.joining(EXTEND_COMMON_SEPARATOR));
        resource.setExtendInfoByKey(GeneralDbConstant.EXTEND_RELATED_HOST_IPS, hostIps);
        String hostUuids = sortedHost.stream()
            .map(ProtectedEnvironment::getUuid)
            .collect(Collectors.joining(EXTEND_COMMON_SEPARATOR));
        resource.setExtendInfoByKey(GeneralDbConstant.EXTEND_RELATED_HOST_IDS, hostUuids);
    }

    private List<ProtectedResource> queryAllResources(Map<String, Object> conditions) {
        List<ProtectedResource> resourceList = new ArrayList<>();
        int pageSize = 500;
        int pageNo = 0;
        ResourceQueryParams params = new ResourceQueryParams();
        params.setShouldIgnoreOwner(true);
        params.setShouldLoadEnvironment(false);
        params.setPage(pageNo);
        params.setSize(pageSize);
        params.setConditions(conditions);
        List<ProtectedResource> records;
        do {
            records = resourceService.query(params).getRecords();
            resourceList.addAll(records);
            params.setPage(++pageNo);
        } while (records.size() == pageSize);
        return resourceList;
    }

    private boolean isResourceInDbContainsHost(ProtectedResource resourceInDb, List<ProtectedEnvironment> hosts) {
        // uuid校验
        String resourceExtendHostUuids = resourceInDb.getExtendInfoByKey(GeneralDbConstant.EXTEND_RELATED_HOST_IDS);
        List<String> hostUuids = hosts.stream().map(ProtectedEnvironment::getUuid).collect(Collectors.toList());
        String[] exHostUuids = Optional.ofNullable(resourceExtendHostUuids)
            .map(e -> e.split(EXTEND_COMMON_SEPARATOR))
            .orElse(new String[0]);
        for (String hostUuid : hostUuids) {
            for (String exHostUuid : exHostUuids) {
                if (Objects.equals(hostUuid, exHostUuid)) {
                    return true;
                }
            }
        }
        return false;
    }

    private String getScriptFromResource(ProtectedResource resource) {
        return resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY);
    }

    private void checkDbIsRegisteredInSapHana(ProtectedResource resource) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.RESOURCE_TYPE, ResourceTypeEnum.DATABASE.getType());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        List<ProtectedResource> protectedResources = queryAllResources(conditions);
        if (protectedResources.isEmpty()) {
            return;
        }
        String dbName = resource.getName();
        String customParams = resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_CUSTOM_PARAM);
        String systemId = GeneralDbUtil.getSystemIdFromCustomParams(customParams);
        String relatedHostIdStr = resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_RELATED_HOST_IDS);
        List<String> agentIds = Arrays.asList(relatedHostIdStr.split(","));
        for (ProtectedResource tmpResource : protectedResources) {
            // 判断 database name
            if (!dbName.toUpperCase(Locale.ROOT).equals(tmpResource.getName().toUpperCase(Locale.ROOT))) {
                continue;
            }
            // 判断 system id
            String tmpSystemId = tmpResource.getExtendInfoByKey(GeneralDbConstant.SYSTEM_ID);
            if (!systemId.toLowerCase(Locale.ROOT).equals(tmpSystemId.toLowerCase(Locale.ROOT))) {
                continue;
            }
            // 判断 agents
            JSONArray tmpNodesJsonArray = JSONArray.fromObject(tmpResource.getExtendInfoByKey(GeneralDbConstant.NODES));
            List<String> tmpAgentIds = JSONArray.toCollection(tmpNodesJsonArray, ProtectedResource.class)
                .stream()
                .flatMap(StreamUtil.match(ProtectedResource.class))
                .map(ResourceBase::getUuid)
                .collect(Collectors.toList());
            if (!CollectionUtils.intersection(tmpAgentIds, agentIds).isEmpty()) {
                log.error("This general database is registered in sap hana databases, registered resource name: {}, "
                        + "uuid: {}.", tmpResource.getName(), tmpResource.getUuid());
                throw new LegoCheckedException(GeneralDbErrorCode.RESOURCE_IS_REGISTERED,
                        "This general database is registered in sap hana databases.");
            }
        }
    }
}
