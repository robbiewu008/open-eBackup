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
package openbackup.oceanprotect.k8s.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.oceanprotect.k8s.protection.access.constant.K8sConstant;
import openbackup.oceanprotect.k8s.protection.access.constant.K8sExtendInfoKey;
import openbackup.oceanprotect.k8s.protection.access.service.K8sCommonService;

import com.alibaba.fastjson.JSON;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.WorkLoadTypeEnum;
import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ValidateUtil;
import openbackup.system.base.common.utils.ValidationUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.model.request.AdvancedConfigReq;
import openbackup.system.base.sdk.job.model.request.ScParameter;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * k8s恢复provider
 *
 */
@Component
@AllArgsConstructor
@Slf4j
public class K8sRestoreProvider implements RestoreInterceptorProvider {
    private final CopyRestApi copyRestApi;
    private final K8sCommonService commonService;

    /**
     * 创建恢复任务前的前置检测
     *
     * @param task 恢复参数对象
     */
    @Override
    public void restoreTaskCreationPreCheck(RestoreTask task) {
        checkAdvancedConfig(task);
        checkScParameterList(task);
        checkNewLocationRestoreEnvVersion(task);
    }

    private void checkNewLocationRestoreEnvVersion(RestoreTask task) {
        if (!RestoreLocationEnum.NEW.equals(task.getTargetLocation())) {
            log.info("Origin restore skip.");
            return;
        }
        ProtectedResource resource = commonService.queryEnvByCopy(copyRestApi.queryCopyByID(task.getCopyId()))
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                        "Original cluster not exist, check fail."));

        checkClusterType(resource.getExtendInfo().get(K8sExtendInfoKey.CLUSTER_TYPE),
                task.getTargetEnv().getExtendInfo().get(K8sExtendInfoKey.CLUSTER_TYPE));
        checkClusterVersion(resource.getVersion(), task.getTargetEnv().getVersion());
    }

    private void checkClusterVersion(String originalVersion, String targetVersion) {
        log.info("Start to check cluster version, origin version: {}, target version: {}", originalVersion,
                targetVersion);
        if (!isClusterVersionCoincide(originalVersion, targetVersion)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "K8s version is not coincide.");
        }
    }

    private void checkClusterType(String originalClusterType, String targetClusterType) {
        log.info("Start to check cluster type, origin type: {}, target type: {}", originalClusterType,
                targetClusterType);
        if (!isClusterTypeCoincide(originalClusterType, targetClusterType)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "K8s clusterType is illegal.");
        }
    }

    private boolean isClusterVersionCoincide(String originalVersion, String targetVersion) {
        String originalVersionPrefix = extractVersion(originalVersion);
        String targetVersionPrefix = extractVersion(targetVersion);
        return originalVersionPrefix.equals(targetVersionPrefix);
    }

    private String extractVersion(String input) {
        Pattern pattern = Pattern.compile(K8sExtendInfoKey.VERSION_PREFIX_PATTERN);
        Matcher matcher = pattern.matcher(input);
        if (matcher.find()) {
            return matcher.group(1);
        } else {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "No matching version found.");
        }
    }

    private boolean isClusterTypeCoincide(String originalClusterType, String targetClusterType) {
        if (!K8sExtendInfoKey.SUPPORT_CLUSTER_TYPE.contains(targetClusterType)
                || !K8sExtendInfoKey.SUPPORT_CLUSTER_TYPE.contains(originalClusterType)) {
            return false;
        }
        return originalClusterType.equals(targetClusterType);
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON.equalsSubType(object)
                || ResourceSubTypeEnum.KUBERNETES_DATASET_COMMON.equalsSubType(object);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        ProtectedEnvironment targetEnv = BeanTools.copy(task.getTargetEnv(), ProtectedEnvironment::new);
        commonService.addIpRule(targetEnv);
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        setRestoreMode(task, copy);
        setDeployType(task);

        if (ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON.equalsSubType(task.getTargetObject().getSubType())) {
            task.getTargetObject().setParentName(task.getTargetObject().getName());
        }

        // 统计速率
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);

        // 填充恢复路径参数
        fillRestorePathParam(task, copy);

        // 将高级配置填充到targetEnv的extendInfo里
        fillTargetEnvExtendInfo(task);
        commonService.fillReStoreAgentConnectedIps(task.getAgents());
        commonService.fillVpcInfo(task.getAdvanceParams(), task.getTargetObject().getUuid());
        return task;
    }

    @Override
    public void postProcess(RestoreTask task, ProviderJobStatusEnum jobStatus) {
        ProtectedEnvironment targetEnv = BeanTools.copy(task.getTargetEnv(), ProtectedEnvironment::new);
        commonService.deleteIpRule(targetEnv);
    }

    private void fillTargetEnvExtendInfo(RestoreTask task) {
        TaskEnvironment targetEnv = task.getTargetEnv();
        if (Objects.isNull(targetEnv)) {
            targetEnv = new TaskEnvironment();
        }
        Map<String, String> extendInfo = targetEnv.getExtendInfo();
        if (Objects.isNull(extendInfo)) {
            extendInfo = new HashMap<>();
        }
        Map<String, String> advanceParams = task.getAdvanceParams();

        // 填充环境变量参数
        fillEnvConfig(extendInfo, advanceParams);

        // 填充Sc参数
        fillScParameter(extendInfo, advanceParams);
        targetEnv.setExtendInfo(extendInfo);
        task.setTargetEnv(targetEnv);
    }

    private void fillEnvConfig(Map<String, String> extendInfo, Map<String, String> advanceParams) {
        if (advanceParams.containsKey(K8sConstant.IS_ENABLE_CHANGE_ENV)) {
            extendInfo.put(K8sConstant.IS_ENABLE_CHANGE_ENV, advanceParams.get(K8sConstant.IS_ENABLE_CHANGE_ENV));
        }
        if (advanceParams.containsKey(K8sConstant.ADVANCED_CONFIG_REQ_LIST)) {
            extendInfo.put(K8sConstant.ADVANCED_CONFIG_REQ_LIST,
                advanceParams.get(K8sConstant.ADVANCED_CONFIG_REQ_LIST));
        }
    }

    private void fillScParameter(Map<String, String> extendInfo, Map<String, String> advanceParams) {
        if (advanceParams.containsKey(K8sConstant.IS_ENABLE_CHANGE_SC_PARAMETER)) {
            extendInfo.put(K8sConstant.IS_ENABLE_CHANGE_SC_PARAMETER,
                advanceParams.get(K8sConstant.IS_ENABLE_CHANGE_SC_PARAMETER));
        }
        if (advanceParams.containsKey(K8sConstant.SC_PARAMETER_LIST)) {
            extendInfo.put(K8sConstant.SC_PARAMETER_LIST,
                advanceParams.get(K8sConstant.SC_PARAMETER_LIST));
        }
    }

    private void fillRestorePathParam(RestoreTask task, Copy copy) {
        String originBackupId = copy.getOriginBackupId();
        Map<String, String> parametersMap = new HashMap<>();
        parametersMap.put(K8sConstant.ORIGIN_BACKUP_ID, originBackupId);
        task.addParameters(parametersMap);
    }

    private void setDeployType(RestoreTask task) {
        // 部署类型
        Map<String, String> extendInfo = task.getTargetEnv().getExtendInfo();
        extendInfo.put(K8sConstant.DEPLOY_TYPE, K8sConstant.DISTRIBUTED);
    }

    private void setRestoreMode(RestoreTask task, Copy copy) {
        String generatedBy = copy.getGeneratedBy();
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(generatedBy)
                || CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.singletonList(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }

    // 检查k8s恢复任务指定的高级配置 修改环境变量的参数是否正确
    private void checkAdvancedConfig(RestoreTask task) {
        Map<String, String> advanceParams = task.getAdvanceParams();
        if (Objects.isNull(advanceParams) || advanceParams.size() == 0) {
            return;
        }
        String isEnableChangeEnvStr = advanceParams.get(K8sConstant.IS_ENABLE_CHANGE_ENV);
        if (!"true".equalsIgnoreCase(isEnableChangeEnvStr)) {
            return;
        }
        String advancedConfigReqListStr = advanceParams.get(K8sConstant.ADVANCED_CONFIG_REQ_LIST);
        List<AdvancedConfigReq> advancedConfigReqs = JSON.parseArray(advancedConfigReqListStr, AdvancedConfigReq.class);
        if (Objects.isNull(advancedConfigReqs) || advancedConfigReqs.size() == K8sConstant.ADVANCED_CONFIG_MIN_SIZE
            || advancedConfigReqs.size() > K8sConstant.ADVANCED_CONFIG_MAX_SIZE) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Advanced config is wrong!");
        }
        for (AdvancedConfigReq advancedConfigReq : advancedConfigReqs) {
            ValidationUtil.fastFailValidate(advancedConfigReq, "Advanced config is wrong!");
            if (!WorkLoadTypeEnum.contains(advancedConfigReq.getWorkLoadType())) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Advanced config is wrong.");
            }
            checkEnvMap(advancedConfigReq.getEnvMap());
        }
    }

    /**
     * 检查k8s恢复任务指定的高级配置 修改存储类参数是否正确
     *
     * @param task 副本恢复参数对象
     */
    private void checkScParameterList(RestoreTask task) {
        Map<String, String> advanceParams = task.getAdvanceParams();
        if (Objects.isNull(advanceParams) || advanceParams.isEmpty()) {
            return;
        }
        String isEnableChangeScParameter = advanceParams.get(K8sConstant.IS_ENABLE_CHANGE_SC_PARAMETER);
        if (!"true".equalsIgnoreCase(isEnableChangeScParameter)) {
            return;
        }
        String scParameterListStr = advanceParams.get(K8sConstant.SC_PARAMETER_LIST);
        List<ScParameter> scParameterReqs = JSON.parseArray(scParameterListStr, ScParameter.class);
        if (Objects.isNull(scParameterReqs)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Sc Parameter is wrong.");
        }
        for (ScParameter scParameterReq : scParameterReqs) {
            checkParamMap(scParameterReq.getParamMap());
        }
    }

    private void checkEnvMap(List<Map<String, String>> envMap) {
        try {
            for (Map<String, String> env : envMap) {
                for (Map.Entry<String, String> entry : env.entrySet()) {
                    ValidateUtil.checkLength(entry.getValue(),
                        K8sConstant.ADVANCED_CONFIG_PARAM_MIN_SIZE, K8sConstant.ADVANCED_CONFIG_PARAM_MAX_SIZE);
                    ValidateUtil.validate(RegexpConstants.NAME_STR, entry.getValue());
                }
            }
        } catch (IllegalArgumentException | EmeiStorDefaultExceptionHandler e) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Advanced config is wrong.");
        }
    }

    /**
     * 校验存储类参数的参数列表
     *
     * @param paramMap 参数列表
     */
    private void checkParamMap(Map<String, String> paramMap) {
        try {
            for (Map.Entry<String, String> entry : paramMap.entrySet()) {
                    ValidateUtil.checkLength(entry.getValue(),
                        K8sConstant.ADVANCED_CONFIG_PARAM_MIN_SIZE, K8sConstant.ADVANCED_CONFIG_PARAM_MAX_SIZE);
                    ValidateUtil.validate(RegexpConstants.NAME_STR, entry.getValue());
            }
        } catch (IllegalArgumentException | EmeiStorDefaultExceptionHandler e) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Sc Parameter is wrong.");
        }
    }
}
