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
package openbackup.data.access.framework.copy.mng.service.impl;

import com.huawei.oceanprotect.base.cluster.sdk.dto.ClusterRequestInfo;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import com.huawei.oceanprotect.base.cluster.sdk.util.ClusterUriUtil;
import com.huawei.oceanprotect.base.cluster.sdk.util.IpUtil;
import openbackup.system.base.sdk.copy.model.StorageInfo;

import com.alibaba.fastjson.JSON;
import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.fasterxml.jackson.core.type.TypeReference;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dme.AvailableTimeRanges;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.copy.controller.req.CatalogQueryReq;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.data.access.framework.core.dao.CopiesProtectionMapper;
import openbackup.data.access.framework.core.dao.CopyMapper;
import openbackup.data.access.framework.core.entity.CopiesEntity;
import openbackup.data.access.framework.core.entity.CopiesProtectionEntity;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.core.model.CopySummaryResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyCommonInterceptor;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.cluster.TargetClusterRestApi;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyResourceSummary;
import openbackup.system.base.sdk.dee.DeeBaseParseRest;
import openbackup.system.base.sdk.dee.DeeInternalCopyRest;
import openbackup.system.base.sdk.dee.model.CopyCatalogsRequest;
import openbackup.system.base.sdk.dee.model.DownLoadCopyInfo;
import openbackup.system.base.sdk.dee.model.DownloadFilesRequest;
import openbackup.system.base.sdk.dee.model.FineGrainedRestore;
import openbackup.system.base.sdk.dee.model.FinegrainedRestoreCopy;
import openbackup.system.base.sdk.dee.model.RestoreFilesResponse;
import openbackup.system.base.sdk.dee.model.Snapshot;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.util.DefaultRoleHelper;
import openbackup.system.base.util.IdUtil;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Service;

import java.net.URI;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * 副本相关的信息
 *
 */
@Service
@Slf4j
public class CopyServiceImpl implements CopyService {
    private static final String INDEXED = "Indexed";

    private static final String AGGREGATION = "isAggregation";

    private static final String SNAPSHOTS = "snapshots";

    private static final String DEFAULT_STORAGE_POOL = "0";

    private final CopyRestApi copyRestApi;

    private final DeeInternalCopyRest deeInternalCopyRest;

    private final DeeBaseParseRest deeBaseParseRest;

    private final DmeUnifiedRestApi dmeUnifiedRestApi;

    private final ProviderManager providerManager;

    private MemberClusterService memberClusterService;

    private TargetClusterRestApi targetApi;

    @Autowired
    private CopyMapper copyMapper;

    @Autowired
    private CopiesProtectionMapper copiesProtectionMapper;

    @Autowired
    private StorageUnitService storageUnitService;

    @Autowired
    private SessionService sessionService;

    public CopyServiceImpl(
        CopyRestApi copyRestApi,
        DeeInternalCopyRest deeInternalCopyRest,
        DmeUnifiedRestApi dmeUnifiedRestApi,
        DeeBaseParseRest deeBaseParseRest,
        ProviderManager providerManager) {
        this.copyRestApi = copyRestApi;
        this.deeInternalCopyRest = deeInternalCopyRest;
        this.dmeUnifiedRestApi = dmeUnifiedRestApi;
        this.deeBaseParseRest = deeBaseParseRest;
        this.providerManager = providerManager;
    }

    @Autowired
    public void setMemberClusterService(MemberClusterService memberClusterService) {
        this.memberClusterService = memberClusterService;
    }

    @Autowired
    @Qualifier("targetClusterApiWithDmaProxyManagePort")
    public void setTargetApi(TargetClusterRestApi targetApi) {
        this.targetApi = targetApi;
    }

    @Override
    public PageListResponse<FineGrainedRestore> listCopyCatalogsName(String copyId, CatalogQueryReq catalogQueryReq) {
        Copy copy = copyRestApi.queryCopyByID(copyId);
        if (!StringUtils.isEmpty(copy.getDeviceEsn()) && memberClusterService.isNeedForward(copy.getDeviceEsn())) {
            log.info(
                "copy(id:{}) is stored in remote node(esn:{}), try forward request to remote node",
                copyId,
                copy.getDeviceEsn());
            return forwardListCopyCatalogsNameToRemote(copy, catalogQueryReq);
        }
        CopyCatalogsRequest catalogsRequest = new CopyCatalogsRequest();
        BeanUtils.copyProperties(catalogQueryReq, catalogsRequest);
        FinegrainedRestoreCopy fileLevelRecoverCopy = getFileLevelRecoverCopy(copy);
        catalogsRequest.setCopyInfo(fileLevelRecoverCopy);
        log.info(
            "Fine grained restore request path:{}, name :{}",
            catalogQueryReq.getParentPath(),
            catalogQueryReq.getName());

        // 插件差异化处理
        CopyCommonInterceptor provider =
            providerManager.findProvider(CopyCommonInterceptor.class, copy.getResourceSubType(), null);
        if (provider != null) {
            provider.buildCatalogsRequest(copy, catalogsRequest);
        }
        RestoreFilesResponse restoreFilesResponse = deeInternalCopyRest.listCopyCatalogs(catalogsRequest);
        PageListResponse<FineGrainedRestore> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(restoreFilesResponse.getItems());
        pageListResponse.setTotalCount(restoreFilesResponse.getTotal());
        return pageListResponse;
    }

    @Override
    public PageListResponse<FineGrainedRestore> listCopyCatalogs(
        String copyId, String parentPath, Integer pageSize, Integer pageNum, String conditions) {
        Copy copy = copyRestApi.queryCopyByID(copyId);
        if (!StringUtils.isEmpty(copy.getDeviceEsn()) && memberClusterService.isNeedForward(copy.getDeviceEsn())) {
            log.info(
                "copy(id:{}) is stored in remote node(esn:{}), try forward request to remote node",
                copyId,
                copy.getDeviceEsn());
            return forwardToRemote(copy, parentPath, pageSize, pageNum, conditions);
        }
        CopyCatalogsRequest catalogsRequest = new CopyCatalogsRequest();
        catalogsRequest.setParentPath(parentPath);
        catalogsRequest.setPageSize(pageSize);
        catalogsRequest.setPageNum(pageNum);
        catalogsRequest.setConditions(conditions);
        FinegrainedRestoreCopy fileLevelRecoverCopy = getFileLevelRecoverCopy(copy);
        catalogsRequest.setCopyInfo(fileLevelRecoverCopy);
        log.info("Fine grained restore request:{}", JSONObject.fromObject(catalogsRequest));

        // 插件差异化处理
        CopyCommonInterceptor provider =
            providerManager.findProvider(CopyCommonInterceptor.class, copy.getResourceSubType(), null);
        if (provider != null) {
            provider.buildCatalogsRequest(copy, catalogsRequest);
        }

        RestoreFilesResponse restoreFilesResponse = deeInternalCopyRest.listCopyCatalogs(catalogsRequest);
        PageListResponse<FineGrainedRestore> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(restoreFilesResponse.getItems());
        pageListResponse.setTotalCount(restoreFilesResponse.getTotal());
        return pageListResponse;
    }

    private PageListResponse<FineGrainedRestore> forwardToRemote(
        Copy copy, String parentPath, Integer pageSize, Integer pageNum, String conditions) {
        ClusterRequestInfo clusterRequestInfo = memberClusterService.getClusterRequestInfo(copy.getDeviceEsn());
        String ip = IpUtil.getAvailableIp(clusterRequestInfo.getIp());
        URI uri = ClusterUriUtil.buildURI(ip, clusterRequestInfo.getPort());
        Map<String, Object> paramMap = new HashMap<>();
        paramMap.put("parentPath", parentPath);
        paramMap.put("pageSize", pageSize);
        paramMap.put("pageNo", pageNum);
        paramMap.put("conditions", conditions);
        try {
            return targetApi.listCopyCatalogs(uri, clusterRequestInfo.getToken(), copy.getUuid(), paramMap);
        } catch (FeignException | LegoCheckedException | LegoUncheckedException e) {
            log.error("request to remote node(esn:{}) failed", copy.getDeviceEsn(), ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(
                CommonErrorCode.NETWORK_CONNECTION_TIMEOUT,
                "request to remote node failed esn:" + copy.getDeviceEsn());
        }
    }

    private PageListResponse<FineGrainedRestore> forwardListCopyCatalogsNameToRemote(
        Copy copy, CatalogQueryReq catalogQueryReq) {
        ClusterRequestInfo clusterRequestInfo = memberClusterService.getClusterRequestInfo(copy.getDeviceEsn());
        String ip = IpUtil.getAvailableIp(clusterRequestInfo.getIp());
        URI uri = ClusterUriUtil.buildURI(ip, clusterRequestInfo.getPort());
        Map<String, Object> paramMap = JsonUtil.read(catalogQueryReq, new TypeReference<Map<String, Object>>() {});
        try {
            return targetApi.listCopyCatalogsName(uri, clusterRequestInfo.getToken(), copy.getUuid(), paramMap);
        } catch (FeignException | LegoCheckedException | LegoUncheckedException e) {
            log.error("request to remote node(esn:{}) failed", copy.getDeviceEsn(), ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(
                CommonErrorCode.NETWORK_CONNECTION_TIMEOUT,
                "request to remote node failed esn:" + copy.getDeviceEsn());
        }
    }

    @Override
    public PageListResponse<AvailableTimeRanges> listAvailableTimeRanges(
        String resourceId, long startTime, long endTime, int pageSize, int pageNo) {
        if (startTime > endTime) {
            log.error("Query available time ranges startTime: {},endTime: {}", startTime, endTime);
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "startTime or endTime is illegal");
        }
        PageListResponse<AvailableTimeRanges> dmeCopyResponse = dmeUnifiedRestApi.listAvailableTimeRanges(resourceId,
            startTime, endTime, pageSize, pageNo);
        TokenBo.UserBo userBo = TokenBo.get().getUser();
        if (DefaultRoleHelper.isAdminOrAudit(userBo.getId()) || dmeCopyResponse == null
            || VerifyUtil.isEmpty(dmeCopyResponse.getRecords())) {
            return dmeCopyResponse;
        }
        return getAvailableCopiesInDomain(resourceId, dmeCopyResponse);
    }

    private PageListResponse<AvailableTimeRanges> getAvailableCopiesInDomain(String resourceId,
        PageListResponse<AvailableTimeRanges> dmeCopyResponse) {
        Map<String, Object> conditionMap = new HashMap<>();
        conditionMap.put("resource_id", resourceId);
        LambdaQueryWrapper<CopiesEntity> queryWrapper = new LambdaQueryWrapper<>();
        fillCopyResourceCondition(conditionMap, queryWrapper);
        addUserDomainCondition(queryWrapper);
        List<String> copyIdsInDomain = Optional.ofNullable(copyMapper.selectList(queryWrapper))
            .orElse(new ArrayList<>())
            .stream()
            .map(CopiesEntity::getUuid)
            .collect(Collectors.toList());
        List<AvailableTimeRanges> availableCopyIds = dmeCopyResponse.getRecords().stream()
            .filter(copyInfo -> copyIdsInDomain.contains(copyInfo.getCopyId()))
            .collect(Collectors.toList());
        PageListResponse<AvailableTimeRanges> availableTimeRangesPageListResponse = new PageListResponse<>();
        availableTimeRangesPageListResponse.setTotalCount(availableCopyIds.size());
        availableTimeRangesPageListResponse.setRecords(availableCopyIds);
        return availableTimeRangesPageListResponse;
    }

    @Override
    public PageListResponse<CopySummaryResource> listCopyResourceSummary(
        int pageNo, int pageSize, String conditions, String[] orders) {
        Map<String, Object> conditionMap = JSONObject.fromObject(conditions).toMap(Object.class);
        LambdaQueryWrapper<CopiesEntity> lambdaQueryWrapper = new LambdaQueryWrapper();
        fillCopiesParam(conditionMap, lambdaQueryWrapper);
        List<CopiesEntity> copiesEntityList =
            Optional.ofNullable(copyMapper.selectList(lambdaQueryWrapper)).orElse(new ArrayList<>());

        List<CopySummaryResource> result = new ArrayList<>();
        if (CollectionUtils.isNotEmpty(copiesEntityList)) {
            // 根据资源id分组
            Map<String, List<CopiesEntity>> copiesMap =
                copiesEntityList.stream().collect(Collectors.groupingBy(CopiesEntity::getResourceId));
            List<String> resourceIds = copiesMap.keySet().stream().collect(Collectors.toList());

            List<CopiesProtectionEntity> protectionEntities = copiesProtectionMapper.selectBatchIds(resourceIds);
            Map<String, CopiesProtectionEntity> protectionEntityMap =
                protectionEntities.stream()
                    .collect(
                        Collectors.toMap(
                            CopiesProtectionEntity::getProtectedResourceId, Function.identity()));
            result =
                copiesMap.keySet().stream()
                    .filter(resourceId -> isSameProtectedSlaId(resourceId, conditionMap, protectionEntityMap))
                    .map(
                        resourceId -> {
                            CopySummaryResource copyResourceSummary = new CopySummaryResource();
                            CopiesProtectionEntity protection = protectionEntityMap.get(resourceId);
                            if (protection != null) {
                                BeanUtils.copyProperties(protection, copyResourceSummary);
                                copyResourceSummary.setIsProtected(protection.getProtectedStatus());
                            } else {
                                copyResourceSummary.setIsProtected(false);
                            }
                            List<CopiesEntity> copiesEntities =
                                copiesMap.get(resourceId).stream()
                                    .sorted(
                                        Comparator.comparing(
                                            copiesEntity ->
                                                copiesEntity
                                                    .getDisplayTimestamp()
                                                    .getTime()))
                                    .collect(Collectors.toList());
                            copyResourceSummary.setCopyCount(copiesEntities.size());
                            BeanUtils.copyProperties(copiesEntities.get(0), copyResourceSummary);
                            return copyResourceSummary;
                        })
                    .collect(Collectors.toList());
            result = filterByCopiesProtection(conditionMap, result);
            result = sortCopySummaryResult(orders, result);
        }

        return convertToPage(pageNo, pageSize, result);
    }

    private boolean isSameProtectedSlaId(
        String resourceId,
        Map<String, Object> conditionMap,
        Map<String, CopiesProtectionEntity> protectionEntityMap) {
        if (!conditionMap.containsKey("protectedSlaId")) {
            return true;
        }
        if (VerifyUtil.isEmpty(protectionEntityMap.get(resourceId))) {
            return false;
        }
        return protectionEntityMap
            .get(resourceId)
            .getProtectedSlaId()
            .equals(conditionMap.get("protectedSlaId").toString());
    }

    @Override
    public CopyResourceSummary queryCopyResourceSummary(String resourceId) {
        Map<String, Object> condition = new HashMap<>();
        condition.put("resource_id", resourceId);
        PageListResponse<CopySummaryResource> response =
            listCopyResourceSummary(0, 1, JSON.toJSONString(condition), null);
        CopySummaryResource copySummaryResource = response.getTotalCount() > 0 ? response.getRecords().get(0) : null;
        CopyResourceSummary summary = new CopyResourceSummary();
        if (copySummaryResource != null) {
            BeanUtils.copyProperties(copySummaryResource, summary);
        }
        return summary;
    }

    private PageListResponse<CopySummaryResource> convertToPage(
        int pageNo, int pageSize, List<CopySummaryResource> result) {
        PageListResponse<CopySummaryResource> pageListResponse = new PageListResponse<>();
        int totalPages = result.size() % pageSize == 0 ? result.size() / pageSize : result.size() / pageSize + 1;
        pageListResponse.setTotalCount(result.size());
        pageListResponse.setPageSize(pageSize);
        pageListResponse.setTotalPages(totalPages);
        pageListResponse.setStartIndex(pageNo);
        int endSize = (pageNo + 1) * pageSize;
        endSize = endSize > result.size() ? result.size() : endSize;
        pageListResponse.setRecords(result.subList(pageNo * pageSize, endSize));
        return pageListResponse;
    }

    private List<CopySummaryResource> filterByCopiesProtection(
        Map<String, Object> conditionMap, List<CopySummaryResource> summaryResources) {
        List<CopySummaryResource> result = summaryResources;
        if (conditionMap.containsKey("protectedSlaName")
            && StringUtils.isNotEmpty(conditionMap.get("protectedSlaName").toString())) {
            result =
                summaryResources.stream()
                    .filter(
                        copySummaryResource ->
                            !VerifyUtil.isEmpty(copySummaryResource.getProtectedSlaName()))
                    .filter(
                        copyResourceSummary ->
                            copyResourceSummary
                                .getProtectedSlaName()
                                .contains(conditionMap.get("protectedSlaName").toString()))
                    .collect(Collectors.toList());
        }
        // 根据状态过滤，根据数量排序，根据查询字段排序，分页
        if (conditionMap.containsKey("protectedStatus")) {
            List<Boolean> protectedStatus = (List<Boolean>) conditionMap.get("protectedStatus");
            result =
                summaryResources.stream()
                    .filter(copyResourceSummary -> containStatus(protectedStatus, copyResourceSummary))
                    .collect(Collectors.toList());
        }
        return result;
    }

    private boolean containStatus(List<Boolean> protectedStatus, CopySummaryResource copyResourceSummary) {
        for (Boolean isProtected : protectedStatus) {
            Boolean isContainStatus =
                isProtected == null
                    ? copyResourceSummary.getIsProtected() == isProtected
                    : isProtected.equals(copyResourceSummary.getIsProtected());
            if (isContainStatus) {
                return true;
            }
        }
        return false;
    }

    private void fillCopiesParam(
        Map<String, Object> conditionMap, LambdaQueryWrapper<CopiesEntity> lambdaQueryWrapper) {
        fillCopyResourceCondition(conditionMap, lambdaQueryWrapper);
        fillCopyPropertyCondition(conditionMap, lambdaQueryWrapper);
        try {
            TokenBo.UserBo userBo = sessionService.getCurrentUser();
            if (userBo != null && !DefaultRoleHelper.isAdmin(userBo.getId())) {
                addUserDomainCondition(lambdaQueryWrapper);
            }
        } catch (LegoCheckedException exception) {
            log.error("get user bo failed");
        }
    }

    private void addUserDomainCondition(LambdaQueryWrapper<CopiesEntity> wrapper) {
        TokenBo.UserBo user = sessionService.getCurrentUser();
        if (!VerifyUtil.isEmpty(user) && !VerifyUtil.isEmpty(user.getDomainId())) {
            if (!IdUtil.isUUID(user.getDomainId())) {
                throw new LegoCheckedException(ErrorCodeConstant.ERR_PARAM, "user domain id incorrect");
            }
            wrapper.inSql(CopiesEntity::getUuid,
                "select resource_object_id from t_domain_r_resource_object " + "where domain_id = '"
                    + user.getDomainId() + "' and type='" + ResourceSetTypeEnum.COPY.getType() + "'");
        }
    }

    private void fillCopyResourceCondition(
        Map<String, Object> conditionMap, LambdaQueryWrapper<CopiesEntity> lambdaQueryWrapper) {
        if (conditionMap.containsKey("resourceSubType") && conditionMap.get("resourceSubType") instanceof JSONArray) {
            JSONArray subTypes = (JSONArray) conditionMap.get("resourceSubType");
            List<String> subTypeList =
                Arrays.stream(subTypes.toArray()).map(o -> o.toString()).collect(Collectors.toList());
            lambdaQueryWrapper.in(CopiesEntity::getResourceSubType, subTypeList);
        }
        if (conditionMap.containsKey("resourceName")) {
            lambdaQueryWrapper.like(CopiesEntity::getResourceName, "%" + conditionMap.get("resourceName") + "%");
        }
        fillCopyResourceIdParams(conditionMap, lambdaQueryWrapper);
        if (conditionMap.containsKey("resourceLocation")) {
            lambdaQueryWrapper.like(
                CopiesEntity::getResourceLocation, "%" + conditionMap.get("resourceLocation") + "%");
        }
        if (conditionMap.containsKey("resourceStatus") && conditionMap.get("resourceStatus") instanceof JSONArray) {
            JSONArray resourceStatuses = (JSONArray) conditionMap.get("resourceStatus");
            List<String> resourceStatusList =
                Arrays.stream(resourceStatuses.toArray()).map(o -> o.toString()).collect(Collectors.toList());
            lambdaQueryWrapper.in(CopiesEntity::getResourceStatus, resourceStatusList);
        }
        if (conditionMap.containsKey("resourceEnvironmentIp")) {
            lambdaQueryWrapper.like(
                CopiesEntity::getResourceEnvironmentIp, "%" + conditionMap.get("resourceEnvironmentIp") + "%");
        }
        if (conditionMap.containsKey("resourceEnvironmentName")) {
            lambdaQueryWrapper.like(
                CopiesEntity::getResourceEnvironmentName, "%" + conditionMap.get("resourceEnvironmentName") + "%");
        }
    }

    private void fillCopyPropertyCondition(
        Map<String, Object> conditionMap, LambdaQueryWrapper<CopiesEntity> lambdaQueryWrapper) {
        if (conditionMap.containsKey("indexed") && StringUtils.isNotEmpty(conditionMap.get("indexed").toString())) {
            lambdaQueryWrapper.eq(CopiesEntity::getIndexed, conditionMap.get("indexed"));
        }
        if (conditionMap.containsKey("gn_range") && StringUtils.isNotEmpty(conditionMap.get("gn_range").toString())) {
            JSONObject gnRange = JSONObject.fromObject(conditionMap);
            JSONArray gnRangeArray = gnRange.getJSONArray("gn_range");
            lambdaQueryWrapper.between(CopiesEntity::getGn, gnRangeArray.get(0), gnRangeArray.get(1));
        }
        if (conditionMap.containsKey("device_esn")
            && StringUtils.isNotEmpty(conditionMap.get("device_esn").toString())) {
            lambdaQueryWrapper.eq(CopiesEntity::getDeviceEsn, conditionMap.get("device_esn"));
        }
        if (conditionMap.containsKey("chain_id") && StringUtils.isNotEmpty(conditionMap.get("chain_id").toString())) {
            lambdaQueryWrapper.eq(CopiesEntity::getChainId, conditionMap.get("chain_id"));
        }
        if (conditionMap.containsKey("generated_by")
            && StringUtils.isNotEmpty(conditionMap.get("generated_by").toString())) {
            lambdaQueryWrapper.eq(CopiesEntity::getGeneratedBy, conditionMap.get("generated_by"));
        }

        if (conditionMap.containsKey("generated_by_array")
            && conditionMap.get("generated_by_array") instanceof JSONArray) {
            JSONArray generatedByArrays = (JSONArray) conditionMap.get("generated_by_array");
            List<String> generatedByArrayList =
                Arrays.stream(generatedByArrays.toArray()).map(o -> o.toString()).collect(Collectors.toList());
            lambdaQueryWrapper.in(CopiesEntity::getGeneratedBy, generatedByArrayList);
        }
    }

    private void fillCopyResourceIdParams(
        Map<String, Object> conditionMap, LambdaQueryWrapper<CopiesEntity> lambdaQueryWrapper) {
        if (conditionMap.containsKey("resourceId")) {
            lambdaQueryWrapper.eq(CopiesEntity::getResourceId, conditionMap.get("resourceId"));
        } else {
            if (conditionMap.containsKey("resource_id")) {
                lambdaQueryWrapper.eq(CopiesEntity::getResourceId, conditionMap.get("resource_id"));
            }
        }
        if (conditionMap.containsKey("resourceIds") && conditionMap.get("resourceIds") instanceof JSONArray) {
            JSONArray resourceIds = (JSONArray) conditionMap.get("resourceIds");
            List<String> resourceIdList =
                Arrays.stream(resourceIds.toArray()).map(o -> o.toString()).collect(Collectors.toList());
            if (!VerifyUtil.isEmpty(resourceIdList)) {
                lambdaQueryWrapper.in(CopiesEntity::getResourceId, resourceIdList);
            }
        }
    }

    private List<CopySummaryResource> sortCopySummaryResult(
        String[] orders, List<CopySummaryResource> summaryResources) {
        List<CopySummaryResource> result = summaryResources;
        if (orders == null) {
            return result;
        }
        for (String order : orders) {
            if (StringUtils.isEmpty(order)) {
                continue;
            }
            if (order.contains("copyCount")) {
                boolean isAsc = order.startsWith("+");
                if (isAsc) {
                    result =
                        summaryResources.stream()
                            .sorted(Comparator.comparing(CopySummaryResource::getCopyCount))
                            .collect(Collectors.toList());
                } else {
                    result =
                        summaryResources.stream()
                            .sorted(
                                Comparator.comparing(
                                    copyResourceSummary -> -copyResourceSummary.getCopyCount()))
                            .collect(Collectors.toList());
                }
            }
        }
        return result;
    }

    @Override
    public String downloadFiles(String copyId, List<String> paths, String recordId) {
        DownloadFilesRequest downloadFilesRequest = new DownloadFilesRequest();
        String requestId = UUID.randomUUID().toString();
        downloadFilesRequest.setRequestId(requestId);
        downloadFilesRequest.setPaths(paths);
        downloadFilesRequest.setRecordId(recordId);
        DownLoadCopyInfo downLoadCopyInfo = new DownLoadCopyInfo();
        Copy copy = copyRestApi.queryCopyByID(copyId);
        JSONObject propertiesJson = JSONObject.fromObject(copy.getProperties());
        JSONArray snapshotsJsonArray = propertiesJson.getJSONArray(SNAPSHOTS);
        List<Snapshot> snapshots = null;
        if (snapshotsJsonArray != null) {
            snapshots = JSONArray.toCollection(snapshotsJsonArray, Snapshot.class);
        }
        downLoadCopyInfo.setSnapshots(snapshots);
        downLoadCopyInfo.setResourceSubType(copy.getResourceSubType());
        downLoadCopyInfo.setUuid(copy.getUuid());
        downLoadCopyInfo.setUserId(copy.getUserId());
        // 下发DEE参数，适配软硬解耦
        Optional<StorageUnitVo> storageUnitVoOptional = storageUnitService.getStorageUnitById(copy.getStorageUnitId());
        if (storageUnitVoOptional.isPresent()) {
            StorageUnitVo storageUnitVo = storageUnitVoOptional.get();
            downLoadCopyInfo.setStorageId(storageUnitVo.getPoolId());
            downLoadCopyInfo.setDeviceId(storageUnitVo.getDeviceId());
        } else {
            log.error("can not find storage unit, storage unit id : {}", copy.getStorageUnitId());
            throw new LegoCheckedException("can not find storage unit, storage unit id : " + copy.getStorageUnitId());
        }
        downloadFilesRequest.setCopyInfo(downLoadCopyInfo);
        log.info("down load files request:{}", JSONObject.fromObject(downloadFilesRequest));
        deeBaseParseRest.downloadFiles(downloadFilesRequest);
        return requestId;
    }

    private FinegrainedRestoreCopy getFileLevelRecoverCopy(Copy copy) {
        FinegrainedRestoreCopy fileLevelRecoverCopy = new FinegrainedRestoreCopy();
        fileLevelRecoverCopy.setGn(copy.getGn());
        fileLevelRecoverCopy.setResourceId(copy.getResourceId());
        fileLevelRecoverCopy.setUuid(copy.getUuid());
        fileLevelRecoverCopy.setGeneratedBy(copy.getGeneratedBy());
        fileLevelRecoverCopy.setChainId(copy.getChainId());
        fileLevelRecoverCopy.setResourceSubType(copy.getResourceSubType());
        fileLevelRecoverCopy.setUserId(copy.getUserId());
        // 下发DEE参数，适配软硬解耦
        Optional<StorageUnitVo> storageUnitVoOptional = storageUnitService.getStorageUnitById(copy.getStorageUnitId());
        if (storageUnitVoOptional.isPresent()) {
            StorageUnitVo storageUnitVo = storageUnitVoOptional.get();
            fileLevelRecoverCopy.setDeviceEsn(storageUnitVo.getDeviceId());
            fileLevelRecoverCopy.setStorageId(storageUnitVo.getPoolId());
        } else {
            // 取默认值
            fileLevelRecoverCopy.setStorageId(DEFAULT_STORAGE_POOL);
            fileLevelRecoverCopy.setDeviceEsn(copy.getDeviceEsn());
        }
        fileLevelRecoverCopy.setIsIndexed(INDEXED.equals(copy.getIndexed()));
        JSONObject propertiesJson = JSONObject.fromObject(copy.getProperties());
        fileLevelRecoverCopy.setIsAggregation(propertiesJson.getBoolean(AGGREGATION, false));
        JSONArray snapshotsJsonArray = propertiesJson.getJSONArray(SNAPSHOTS);
        List<Snapshot> snapshots = null;
        if (snapshotsJsonArray != null) {
            snapshots = JSONArray.toCollection(snapshotsJsonArray, Snapshot.class);
        }
        fileLevelRecoverCopy.setSnapshots(snapshots);
        return fileLevelRecoverCopy;
    }

    @Override
    public BasePage<Copy> queryCopiesByResourceIdAndStatusAndExtendType(
        String resourceId, String status, String extendType, int retryNum) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("resource_id", resourceId);
        conditions.put("status", status);
        conditions.put("extend_type", extendType);
        return copyRestApi.queryCopies(0, retryNum, conditions, null);
    }

    @Override
    public StorageInfo getStorageInfo(String copyId) {
        StorageInfo storageInfo = new StorageInfo();
        Copy copy = copyRestApi.queryCopyByID(copyId);
        if (VerifyUtil.isEmpty(copy)) {
            log.warn("can not find copy by copyId : {}", copyId);
        } else {
            Optional<StorageUnitVo> storageUnitVoOptional =
                storageUnitService.getStorageUnitById(copy.getStorageUnitId());
            if (storageUnitVoOptional.isPresent()) {
                StorageUnitVo storageUnitVo = storageUnitVoOptional.get();
                storageInfo.setStoragePool(storageUnitVo.getPoolId());
                storageInfo.setDeviceId(storageUnitVo.getDeviceId());
                storageInfo.setDeviceType(storageUnitVo.getDeviceType());
            } else {
                log.warn("can not find storage unit, storage unit id : {}", copy.getStorageUnitId());
            }
        }
        return storageInfo;
    }

    @Override
    public void closeCopyGuestSystem(String copyId) {
        deeBaseParseRest.closeCopyGuestSystem(copyId);
    }
}
