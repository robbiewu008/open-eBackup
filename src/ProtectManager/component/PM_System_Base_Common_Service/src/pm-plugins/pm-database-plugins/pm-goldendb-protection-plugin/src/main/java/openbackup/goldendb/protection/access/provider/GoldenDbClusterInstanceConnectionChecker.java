package openbackup.goldendb.protection.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.goldendb.protection.access.constant.GoldenDbConstant;
import openbackup.goldendb.protection.access.dto.instance.Gtm;
import openbackup.goldendb.protection.access.dto.instance.MysqlNode;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 * @author s30036254
 * @since 2023-02-17
 */
@Component
@Slf4j
public class GoldenDbClusterInstanceConnectionChecker extends UnifiedResourceConnectionChecker {
    private final GoldenDbService goldenDbService;

    private final GoldenDbClusterConnectionChecker goldenDbClusterConnectionChecker;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param goldenDbService goldenDbService
     * @param goldenDbClusterConnectionChecker 集群检查接口
     */
    public GoldenDbClusterInstanceConnectionChecker(
        final ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        final AgentUnifiedService agentUnifiedService, GoldenDbService goldenDbService,
        final GoldenDbClusterConnectionChecker goldenDbClusterConnectionChecker) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.goldenDbService = goldenDbService;
        this.goldenDbClusterConnectionChecker = goldenDbClusterConnectionChecker;
    }

    /**
     * 连通性子类过滤接口
     *
     * @param object 受保护资源
     * @return boolean 是否使用该类进行连通性校验
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object)
            && ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType().equals(object.getSubType());
    }

    /**
     * 获取集群节点及其对应的主机
     *
     * @param environment 集群
     * @return key为集群节点，value为集群节点对应的主机列表
     */
    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>>
        collectConnectableResources(ProtectedResource environment) {
        log.info("GoldenDB cluster instance ConnectionChecker,collectConnectableResources,environment.uuid: {}",
            environment.getUuid());
        Authentication auth = environment.getAuth();
        Map<ProtectedResource, List<ProtectedEnvironment>> nodeHostMap = new LinkedHashMap<>();
        List<MysqlNode> computeNodes =
            Optional.ofNullable(goldenDbService.getComputeNode(BeanTools.copy(environment, ProtectedEnvironment::new)))
                .orElseGet(Collections::emptyList);
        List<Gtm> gtms =
            Optional.ofNullable(goldenDbService.getGtmNode(BeanTools.copy(environment, ProtectedEnvironment::new)))
                .orElseGet(Collections::emptyList);
        collectComputeResource(auth, nodeHostMap, computeNodes);
        collectGtmResource(auth, nodeHostMap, gtms);
        ProtectedEnvironment cluster = goldenDbService.getEnvironmentById(environment.getParentUuid());
        Map<ProtectedResource, List<ProtectedEnvironment>> clusterMap =
            goldenDbClusterConnectionChecker.collectConnectableResources(cluster);
        nodeHostMap.putAll(clusterMap);
        return nodeHostMap;
    }

    private void collectGtmResource(Authentication auth, Map<ProtectedResource, List<ProtectedEnvironment>> nodeHostMap,
        List<Gtm> gtms) {
        gtms.stream().forEach(gtm -> {
            ProtectedResource protectedResource = new ProtectedResource();
            protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
            protectedResource.setSubType(ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType());
            HashMap<String, String> extendInfo = new HashMap<>();
            collectGtmResourcePart(gtm, extendInfo);
            protectedResource.setExtendInfo(extendInfo);
            protectedResource.setAuth(auth);
            ProtectedEnvironment agentEnvironment = goldenDbService.getEnvironmentById(gtm.getParentUuid());
            nodeHostMap.put(protectedResource,
                Lists.newArrayList(BeanTools.copy(agentEnvironment, ProtectedEnvironment::new)));
        });
    }

    private void collectGtmResourcePart(Gtm gtm, HashMap<String, String> extendInfo) {
        for (Field field : gtm.getClass().getDeclaredFields()) {
            field.setAccessible(true);
            try {
                if (field.get(gtm) != null) {
                    extendInfo.put(field.getName(), field.get(gtm).toString());
                }
            } catch (IllegalAccessException e) {
                log.error("can not get param from gtmNode");
            }
        }
    }

    private void collectComputeResource(Authentication auth,
        Map<ProtectedResource, List<ProtectedEnvironment>> nodeHostMap, List<MysqlNode> computeNodes) {
        computeNodes.stream().forEach(mysqlNode -> {
            ProtectedResource protectedResource = new ProtectedResource();
            protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
            protectedResource.setSubType(ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType());
            HashMap<String, String> extendInfo = new HashMap<>();
            collectComputeResourcePart(mysqlNode, extendInfo);
            protectedResource.setExtendInfo(extendInfo);
            protectedResource.setAuth(auth);
            ProtectedEnvironment agentEnvironment = goldenDbService.getEnvironmentById(mysqlNode.getParentUuid());
            nodeHostMap.put(protectedResource,
                Lists.newArrayList(BeanTools.copy(agentEnvironment, ProtectedEnvironment::new)));
        });
    }

    private void collectComputeResourcePart(MysqlNode mysqlNode, HashMap<String, String> extendInfo) {
        for (Field field : mysqlNode.getClass().getDeclaredFields()) {
            field.setAccessible(true);
            try {
                if (field.get(mysqlNode) != null) {
                    extendInfo.put(field.getName(), field.get(mysqlNode).toString());
                }
            } catch (IllegalAccessException e) {
                log.error("can not get param from computeNode");
            }
        }
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        log.info("To deal with goldenDB collectActionResults");

        // 父类获得的结果
        List<ActionResult> results = super.collectActionResults(checkReport, context);

        // 插件指定要处理的异常
        List<ActionResult> mismatchResult =
            results.stream().filter(it -> it.getCode() == GoldenDbConstant.NODE_TYPE_MISMATCH).map(it -> {
                ActionResult actionResult = new ActionResult();
                actionResult.setCode(it.getCode());
                actionResult.setMessage(it.getMessage());
                actionResult.setBodyErr(it.getBodyErr());
                log.info("get plugin error information {}", it.getMessage());
                actionResult.setDetailParams(getParameters(it.getMessage()));
                return actionResult;
            }).collect(Collectors.toList());
        log.info("goldenDB node type mismatch size is {}", mismatchResult.size());
        List<ActionResult> missComponentResult =
            results.stream().filter(it -> it.getCode() == GoldenDbConstant.MISS_COMPONENT).map(it -> {
                ActionResult actionResult = new ActionResult();
                actionResult.setCode(it.getCode());
                actionResult.setMessage(it.getMessage());
                actionResult.setBodyErr(it.getBodyErr());
                log.info("get plugin error information {}", it.getMessage());
                HashMap hash = JsonUtil.read(it.getMessage(), HashMap.class);
                List<String> pluginParameter = (List<String>) hash.get(GoldenDbConstant.PARAMETERS);
                actionResult.setDetailParams(pluginParameter);
                return actionResult;
            }).collect(Collectors.toList());
        log.info("goldenDB node type misComponent size is {}", missComponentResult.size());

        // 插件没有指定要处理的异常
        List<ActionResult> commonResults = results.stream()
            .filter(it -> it.getCode() != GoldenDbConstant.NODE_TYPE_MISMATCH
                && it.getCode() != GoldenDbConstant.MISS_COMPONENT)
            .collect(Collectors.toList());
        commonResults.addAll(mismatchResult);
        commonResults.addAll(missComponentResult);
        return commonResults;
    }

    private List<String> getParameters(String message) {
        Map read = JsonUtil.read(message, Map.class);
        String parameters = String.valueOf(read.get(GoldenDbConstant.PARAMETERS));
        String[] split = parameters.substring(1, parameters.length() - 1).split(GoldenDbConstant.SEPARATOR);

        // 从插件返回的信息拼凑错误码的参数
        String user = split[LegoNumberConstant.ZERO];
        String nodeType = split[LegoNumberConstant.ONE];
        String agentUuid = split[LegoNumberConstant.TWO];
        String dataType =
            nodeType.substring(0, nodeType.length() - GoldenDbConstant.SUB_NODE_LENGTH).toUpperCase(Locale.ENGLISH);
        String endpoint = goldenDbService.getResourceById(agentUuid).getEndpoint();
        log.info("End of obtaining parameters.");
        return Arrays.asList(dataType, endpoint, user, nodeType);
    }
}
