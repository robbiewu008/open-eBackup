/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.gaussdbt.protection.access.provider.checker;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.AbstractResourceChecker;
import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.asserts.PowerAssert;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.function.Consumer;
import java.util.stream.Collectors;

/**
 * GaussDBT资源检查checker
 *
 * @author hwx1144169
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-30
 */
@Slf4j
@Component("gaussDBTResourceChecker")
public class GaussDBTResourceChecker extends AbstractResourceChecker<AppEnvResponse> {
    private final UnifiedResourceConnectionChecker connectionChecker;

    private final UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker;

    /**
     * 构造方法
     *
     * @param environmentRetrievalsService 插件配置管理
     * @param connectionChecker 连通性检查对象
     * @param clusterIntegrityChecker 集群完整性检查
     */
    public GaussDBTResourceChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        @Qualifier("unifiedResourceConnectionChecker") UnifiedResourceConnectionChecker connectionChecker,
        @Qualifier("unifiedClusterResourceIntegrityChecker")
            UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker) {
        super(environmentRetrievalsService);
        this.connectionChecker = connectionChecker;
        this.clusterIntegrityChecker = clusterIntegrityChecker;
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.GAUSSDBT.getType().equals(protectedResource.getSubType());
    }

    /**
     * 生成检查结果
     *
     * @param protectedResource 需要检查的资源
     * @return checkResult 检查结果
     */
    @Override
    public CheckResult<AppEnvResponse> generateCheckResult(ProtectedResource protectedResource) {
        // 检测连通性并收集集群信息
        CheckResult<Object> connectionResult = connectionChecker.generateCheckResult(protectedResource);
        CheckResult<AppEnvResponse> integrityResult = new CheckResult<>();
        try {
            integrityResult = clusterIntegrityChecker.generateCheckResult(protectedResource);
        } catch (LegoCheckedException e) {
            integrityResult.setEnvironment(protectedResource.getEnvironment());
            log.error("generate GaussDBT integrity result failed. protectedResource:{}", protectedResource.getUuid());
        }
        integrityResult.setResults(connectionResult.getResults());
        return integrityResult;
    }

    /**
     * 收集检查结果
     *
     * @param checkReports 检查报告
     * @param context 上下文
     * @return list 最终结果
     */
    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<AppEnvResponse>> checkReports,
        Map<String, Object> context) {
        // 获取所有检查结果
        List<CheckResult<AppEnvResponse>> checkResults = checkReports.stream()
            .map(CheckReport::getResults).flatMap(List::stream).collect(Collectors.toList());
        // 获取ActionResults
        List<ActionResult> failedResults = Lists.newArrayList();
        List<ActionResult> successResults = checkResults.stream()
            .map(CheckResult::getResults)
            .peek(pickFailedResults(failedResults))
            .filter(actionResult -> actionResult.getCode() == ActionResult.SUCCESS_CODE)
            .collect(Collectors.toList());
        if (successResults.isEmpty()) {
            log.error("GaussDBT nodes check connection failed.");
            return failedResults;
        }
        // 判断AppEnvResponse是不是为空，不为空则加到上下文中，注册时候使用
        List<AppEnvResponse> appEnvResponses = checkResults.stream()
            .map(CheckResult::getData).filter(Objects::nonNull).collect(Collectors.toList());
        PowerAssert.notEmpty(appEnvResponses,
            () -> new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED, "cluster nodes query failed."));
        context.put(GaussDBTConstant.CLUSTER_INFO_KEY, JSONObject.writeValueAsString(appEnvResponses));
        return successResults;
    }

    private Consumer<ActionResult> pickFailedResults(List<ActionResult> failedResults) {
        return actionResult -> {
            if (actionResult.getCode() != ActionResult.SUCCESS_CODE) {
                failedResults.add(actionResult);
            }
        };
    }
}
