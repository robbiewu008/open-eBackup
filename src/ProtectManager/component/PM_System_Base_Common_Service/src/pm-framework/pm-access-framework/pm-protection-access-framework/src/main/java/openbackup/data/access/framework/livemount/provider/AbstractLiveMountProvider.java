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
package openbackup.data.access.framework.livemount.provider;

import openbackup.data.access.client.sdk.api.framework.dee.DeeLiveMountRestApi;
import openbackup.data.access.client.sdk.api.framework.dee.model.OcLiveMountCloneReq;
import openbackup.data.access.client.sdk.api.framework.dee.model.OcLiveMountTaskReq;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.CopyIndexConstants;
import openbackup.data.access.framework.livemount.common.model.LiveMountCloneRequest;
import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.access.framework.livemount.service.impl.PerformanceValidator;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.RemotePath;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryRoleEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountModifyParam;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountPerformance;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountRemoveQosParam;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.PageQueryRestApi;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.livemount.model.Performance;
import openbackup.system.base.sdk.resource.EnvironmentRestApi;
import openbackup.system.base.sdk.resource.model.Environment;
import openbackup.system.base.service.DeployTypeService;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.springframework.beans.factory.annotation.Autowired;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * Abstract Live Mount Provider
 *
 */
@Slf4j
public abstract class AbstractLiveMountProvider implements LiveMountProvider {
    /**
     * 后置脚本
     */
    public static final String POST_SCRIPT = "post_script";

    /**
     * 前置脚本
     */
    public static final String PRE_SCRIPT = "pre_script";

    /**
     * 失败后置脚本
     */
    public static final String FAIL_SCRIPT = "failed_script";

    /**
     * windows名称
     */
    public static final String WINDOWS = "windows";

    /**
     * dme备份id
     */
    public static final String BACKUP_ID = "backup_id";

    /**
     * 文件过滤
     */
    public static final String FILE_REGEX = "^\\w+:.*";

    /**
     * windows脚本文件
     */
    public static final String BAT_FILE = ".bat";

    /**
     * shell脚本文件
     */
    public static final String SHELL_FILE = ".sh";

    /**
     * smart qos参数
     */
    public static final String PERFORMANCE = "performance";

    /**
     *  当返回null, 界面显示为--
     */
    private static final String DISPLAY_NULL = "--";


    private static final String TENANT_ID = "tenantId";

    /**
     *  部署类型service
     */
    @Autowired
    protected DeployTypeService deployTypeService;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private PerformanceValidator performanceValidator;

    @Autowired
    private EnvironmentRestApi environmentRestApi;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private DeeLiveMountRestApi deeLiveMountRestApi;

    /**
     * clone copy
     *
     * @param cloneRequest cloneRequest
     * @return copy info
     */
    @Override
    public CopyInfoBo cloneCopy(LiveMountCloneRequest cloneRequest) {
        Copy copy = cloneRequest.getSourceCopy();
        JSONObject properties = JSONObject.fromObject(copy.getProperties());
        String backupId = Optional.ofNullable(properties.getString(BACKUP_ID)).orElse(copy.getUuid());
        String cloneBackupId = cloneRequest.getTargetCopyUuid();
        String resourceSubType = copy.getResourceSubType();
        List<LiveMountFileSystemShareInfo> fileSystemShareInfo = loadLiveMountFileSystemShareInfo(cloneRequest);
        LiveMountPerformance performance = loadLiveMountPerformance(cloneRequest);

        CloneCopyParam cloneCopyParam = new CloneCopyParam(backupId, cloneBackupId, resourceSubType,
            fileSystemShareInfo, performance);
        // 安全一体机适配
        if (deployTypeService.isCyberEngine()) {
            List<StorageRepository> repositories = getCyberEngineCloneCopyParam(copy);
            cloneCopyParam.setRepositories(repositories);
            properties.set(CopyPropertiesKeyConstant.KEY_REPOSITORIES, repositories);
            // 安全一体机适配共享路径恢复：调用DEE接口创建DEE侧共享路径恢复任务
            OcLiveMountTaskReq liveMountTaskReq =
                buildOcLiveMountTaskReq(copy, cloneBackupId, performance);
            deeLiveMountRestApi.createLiveMountTask(liveMountTaskReq);
            log.info("create live mount task[{}] success.", liveMountTaskReq.getTaskId());
            // 调用DEE接口创建克隆
            if (CollectionUtils.isEmpty(fileSystemShareInfo)) {
                log.error("file system share info list is empty.");
                throw new LegoCheckedException(
                    ErrorCodeConstant.ERR_PARAM, "fileSystemShareInfo parameters is invalid");
            }
            OcLiveMountCloneReq liveMountCloneParam =
                buildOcLiveMountCloneParam(copy, cloneBackupId, fileSystemShareInfo, performance);
            // 安全一体机适配共享路径恢复：调用dee内部接口clone文件系统 deeLiveMountRestApi
            deeLiveMountRestApi.createFilesystemClone(liveMountCloneParam);
            log.info("task[{}] create filesystem clone success.", liveMountCloneParam.getTaskId());
        } else {
            cloneBackup(cloneCopyParam);
        }
        log.info("clone backup id: {}", cloneBackupId);

        CopyInfoBo copyInfo = JSONObject.fromObject(copy).toBean(CopyInfoBo.class);
        properties.set(BACKUP_ID, cloneBackupId);
        copyInfo.setUuid(cloneBackupId);
        copyInfo.setProperties(properties.toString());
        return copyInfo;
    }

    private OcLiveMountTaskReq buildOcLiveMountTaskReq(
        Copy copy, String cloneBackupId, LiveMountPerformance performance) {
        OcLiveMountTaskReq liveMountTaskReq = new OcLiveMountTaskReq();
        liveMountTaskReq.setTaskId(cloneBackupId);
        liveMountTaskReq.setRequestId(cloneBackupId);
        liveMountTaskReq.setFileSystemKeepTime(performance.getFileSystemKeepTime());
        JSONObject resourceJsonObj = JSONObject.fromObject(copy.getResourceProperties());
        liveMountTaskReq.setDeviceId(resourceJsonObj.getString("root_uuid"));
        return liveMountTaskReq;
    }

    private OcLiveMountCloneReq buildOcLiveMountCloneParam(Copy copy, String cloneBackupId,
        List<LiveMountFileSystemShareInfo> fileSystemShareInfo, LiveMountPerformance performance) {
        OcLiveMountCloneReq liveMountCloneParam = new OcLiveMountCloneReq();
        liveMountCloneParam.setRequestId(cloneBackupId);
        liveMountCloneParam.setTaskId(cloneBackupId);
        JSONObject resourceJsonObj = JSONObject.fromObject(copy.getResourceProperties());
        liveMountCloneParam.setDeviceId(resourceJsonObj.getString("root_uuid"));
        JSONObject propertiesJsonObj = JSONObject.fromObject(copy.getProperties());
        liveMountCloneParam.setParentFileSystemId(propertiesJsonObj.getString("filesystemId"));
        liveMountCloneParam.setParentFileSystemName(propertiesJsonObj.getString("filesystemName"));
        liveMountCloneParam.setParentSnapshotName(propertiesJsonObj.getString("snapshotName"));
        liveMountCloneParam.setParentSnapshotId(propertiesJsonObj.getString("snapshotId"));
        liveMountCloneParam.setVstoreId(propertiesJsonObj.getString("tenantId"));
        liveMountCloneParam.setName(fileSystemShareInfo.get(0).getFileSystemName());
        return liveMountCloneParam;
    }

    private List<StorageRepository> getCyberEngineCloneCopyParam(Copy copy) {
        log.info("CyberEngine construct StorageRepository");
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(copy.getResourceId());
        Environment targetEnvironment = PageQueryRestApi.get(environmentRestApi::queryEnvironment)
            .queryOne(new JSONObject().set("uuid", resOptional.get().getRootUuid()));
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setType(RepositoryTypeEnum.DATA.getType());
        storageRepository.setRole(RepositoryRoleEnum.SLAVE_ROLE.getRoleType());
        storageRepository.setLocal(Boolean.TRUE);
        Endpoint endpoint = new Endpoint();
        endpoint.setIp(copy.getResourceEnvironmentIp());
        endpoint.setPort(Integer.parseInt(targetEnvironment.getPort()));
        storageRepository.setEndpoint(endpoint);
        storageRepository.setExtendAuth(resOptional.get().getAuth());
        Map<String, Object> extendInfo = new HashMap<>();
        extendInfo.put(StorageRepository.REPOSITORIES_KEY_ENS, resOptional.get().getRootUuid());
        extendInfo.put(ContextConstants.COPY_FORMAT, 1);
        storageRepository.setExtendInfo(extendInfo);
        RemotePath remotePath = new RemotePath();
        remotePath.setId(resOptional.get().getExtendInfo().get("fileSystemId"));
        remotePath.setType(1);
        remotePath.setPath(CopyIndexConstants.FILE_SEPARATOR + resOptional.get().getName());
        JSONObject properties = JSONObject.fromObject(copy.getProperties());
        remotePath.setParentId(properties.getString(TENANT_ID));
        storageRepository.setRemotePath(Collections.singletonList(remotePath));
        storageRepository.setProtocol(1);
        return Collections.singletonList(storageRepository);
    }

    private List<LiveMountFileSystemShareInfo> loadLiveMountFileSystemShareInfo(LiveMountCloneRequest cloneRequest) {
        LiveMountEntity liveMountEntity = cloneRequest.getLiveMountEntity();
        if (liveMountEntity == null) {
            return Collections.emptyList();
        }
        String fileSystemShareInfo = liveMountEntity.getFileSystemShareInfo();
        if (fileSystemShareInfo == null) {
            return Collections.emptyList();
        }
        return JSONArray.fromObject(fileSystemShareInfo).toBean(LiveMountFileSystemShareInfo.class);
    }

    private LiveMountPerformance loadLiveMountPerformance(LiveMountCloneRequest cloneRequest) {
        return loadPerformance(cloneRequest.getLiveMountEntity())
                .map(performance -> JSONObject.fromObject(performance).toBean(LiveMountPerformance.class))
                .orElse(null);
    }

    /**
     * load performance from parameter
     *
     * @param liveMountEntity live mount entity
     * @return performance
     */
    protected Optional<Performance> loadPerformance(LiveMountEntity liveMountEntity) {
        if (liveMountEntity == null) {
            return Optional.empty();
        }
        JSONObject parameters = JSONObject.fromObject(liveMountEntity.getParameters());
        Map<String, Object> params = parameters.toMap(Object.class);
        Map<String, Object> performanceParams = JSONObject.fromObject(params.get(PERFORMANCE)).toMap(Object.class);
        return Optional.ofNullable(performanceValidator.loadPerformance(performanceParams));
    }

    /**
     * modify live mount qos
     *
     * @param mountedCopyId mounted copy id
     * @param performanceProperties performance properties
     */
    protected void modifyLiveMountQos(String mountedCopyId, Map<String, Object> performanceProperties) {
        Copy copy = copyRestApi.queryCopyByID(mountedCopyId);
        JSONObject copyProperties = JSONObject.fromObject(copy.getProperties());
        String backupId = copyProperties.getString(BACKUP_ID);
        LiveMountModifyParam param = new LiveMountModifyParam();
        param.setBackupId(backupId);
        param.setAppType(copy.getResourceSubType());
        Performance performance = performanceValidator.loadPerformance(performanceProperties);
        LiveMountPerformance liveMountPerformance = JSONObject.cast(performance, LiveMountPerformance.class);
        param.setPerformance(liveMountPerformance);
        modifyLiveMountQos(param);
    }

    /**
     * modify live mount qos
     *
     * @param mountedCopyId mounted copy id
     */
    protected void removeLiveMountQos(String mountedCopyId) {
        Copy copy = copyRestApi.queryCopyByID(mountedCopyId);
        JSONObject copyProperties = JSONObject.fromObject(copy.getProperties());
        String backupId = copyProperties.getString(BACKUP_ID);
        LiveMountRemoveQosParam param = new LiveMountRemoveQosParam();
        param.setBackupId(backupId);
        param.setAppType(copy.getResourceSubType());
        removeLiveMountQos(param);
    }

    /**
     * check script path and type
     *
     * @param params params
     * @param environmentOsType environment type
     */
    public void checkScriptPathAndType(Map<String, Object> params, String environmentOsType) {
        checkScriptPathAndType(environmentOsType, params, PRE_SCRIPT);
        checkScriptPathAndType(environmentOsType, params, POST_SCRIPT);
        checkScriptPathAndType(environmentOsType, params, FAIL_SCRIPT);
    }

    /**
     * check performance params
     *
     * @param params performance params
     */
    public void checkPerformance(Map<String, Object> params) {
        Map<String, Object> performanceParams = JSONObject.fromObject(params.get(PERFORMANCE)).toMap(Object.class);
        Performance performance = performanceValidator.loadPerformance(performanceParams);

        // 过滤掉无效和0的输入
        JSONObject performanceJsonObject = JSONObject.fromObject(performance);
        Map<String, Object> performanceParams1 = JSONObject.toMap(performanceJsonObject, Object.class);
        if (performanceParams.size() != performanceParams1.size()) {
            throw new LegoCheckedException(ErrorCodeConstant.ERR_PARAM, "performance parameters is invalid");
        }

        if (performance != null) {
            performanceValidator.validatePerformance(performance);
        }
    }

    private void checkScriptPathAndType(String osType, Map<String, Object> parameters, String name) {
        String script = Optional.ofNullable(parameters).map(map -> map.get(name)).map(Object::toString).orElse(null);
        if (VerifyUtil.isEmpty(script)) {
            return;
        }
        if (script.contains("\\") || script.contains("/")) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, name + " must be file name.");
        }
        if (WINDOWS.equals(osType)) {
            if (script.matches(FILE_REGEX)) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, name + " must be file name.");
            }
            if (!script.endsWith(BAT_FILE)) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, name + " must be bat file.");
            }
        } else if (!script.endsWith(SHELL_FILE)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, name + " must be shell script.");
        } else {
            log.warn("Unexpected scene happen.");
        }
    }

    /**
     * has valid smart qos performance param
     *
     * @param performanceProperties performance param
     * @return hasPerformanceParam
     */
    public boolean hasNonePerformanceParam(Map<String, Object> performanceProperties) {
        Performance performance = performanceValidator.loadPerformance(performanceProperties);
        return VerifyUtil.isEmpty(performance);
    }

    /**
     * update performance settings
     *
     * @param liveMountEntity live mount entity
     */
    @Override
    public void updatePerformanceSetting(LiveMountEntity liveMountEntity) {
        String mountedCopyId = liveMountEntity.getMountedCopyId();
        JSONObject liveMountProperties = JSONObject.fromObject(liveMountEntity.getParameters());
        Map<String, Object> performance = liveMountProperties.getJSONObject(PERFORMANCE).toMap(Object.class);
        if (hasNonePerformanceParam(performance)) {
            removeLiveMountQos(mountedCopyId);
        } else {
            modifyLiveMountQos(mountedCopyId, performance);
        }
    }

    /**
     * check source and target environment os type
     *
     * @param sourceEnvironmentData source os type
     * @param targetEnvironmentData target os type
     * @param osType os type name
     */
    public void checkEnvironmentData(Object sourceEnvironmentData, Object targetEnvironmentData, String osType) {
        if (!Objects.equals(sourceEnvironmentData, targetEnvironmentData)) {
            // 判断保护对象操作系统是否为空，为空显示--
            String sourceEnvironmentOsType =
                    VerifyUtil.isEmpty(sourceEnvironmentData) ? DISPLAY_NULL : String.valueOf(sourceEnvironmentData);
            // 判断目标对象操作系统是否为空，为空显示--
            String targetEnvironmentDataOsType =
                    VerifyUtil.isEmpty(targetEnvironmentData) ? DISPLAY_NULL : String.valueOf(targetEnvironmentData);
            throw new LegoCheckedException(
                    CommonErrorCode.SOURCE_TARGET_ENV_INCONSISTENT,
                    new String[] {sourceEnvironmentOsType, targetEnvironmentDataOsType},
                    osType + " of source environment and target environment is inconsistent.");
        }
    }

    /**
     * get config
     *
     * @param data data
     * @param tClass class
     * @param <T> template param
     * @return config
     */
    public static <T> Optional<T> loadConfig(Object data, Class<T> tClass) {
        T vmConfig = JSONObject.cast(data, tClass);
        if (vmConfig == null) {
            return Optional.empty();
        }
        if (JSONObject.fromObject(vmConfig).isEmpty()) {
            return Optional.empty();
        }
        return Optional.of(vmConfig);
    }

    /**
     * clone backup
     *
     * @param cloneCopyParam backup clone param
     */
    protected abstract void cloneBackup(CloneCopyParam cloneCopyParam);

    /**
     * remove live mount qos
     *
     * @param param param
     */
    protected abstract void removeLiveMountQos(LiveMountRemoveQosParam param);

    /**
     * modify live mount qos
     *
     * @param param param
     */
    protected abstract void modifyLiveMountQos(LiveMountModifyParam param);
}
