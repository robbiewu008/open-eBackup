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

import com.huawei.oceanprotect.job.sdk.JobService;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.DmeReplicationRestApi;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.model.RemoveCopyRequest;
import openbackup.data.access.framework.copy.mng.provider.DefaultCopyDeleteInterceptor;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.core.dao.CopyMapper;
import openbackup.data.access.framework.core.entity.CopiesEntity;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.core.model.CopySummaryCount;
import openbackup.data.access.framework.core.model.CopySummaryResource;
import openbackup.data.access.framework.core.model.CopySummaryResourceCondition;
import openbackup.data.access.framework.core.model.CopySummaryResourceQuery;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyDeleteInterceptor;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JSONObjectCovert;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.ReplicatedCopies;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * copy service
 *
 */
@Slf4j
@Service
public class CopyManagerServiceImpl implements CopyManagerService {
    // 排序字段前缀，表示增序
    private static final String ORDER_INCREASE_PREFIX = "+";

    // 排序字段前缀，表示减序
    private static final String ORDER_DECREASE_PREFIX = "-";

    // 副本资源分页查询排序字段，key: 接口查询字段，value：db查询字段。只有定义在这里的值才是有效排序字段。
    private static final Map<String, String> ORDER_FIELD_MAP;

    static {
        ORDER_FIELD_MAP = new HashMap<>();
        ORDER_FIELD_MAP.put("copyCount", "copy_count");
        ORDER_FIELD_MAP.put("displayTimestamp", "display_timestamp");
    }

    private JobService jobService;

    private CopyRestApi copyRestApi;

    private DmeReplicationRestApi dmeReplicationRestApi;

    private ProtectedEnvironmentService protectedEnvironmentService;

    private ProviderManager providerManager;

    private DefaultCopyDeleteInterceptor defaultCopyDeleteInterceptor;

    private CopyMapper copyMapper;

    private final List<String> notifyCopyDeleteTypeList = Arrays.asList(CopyGeneratedByEnum.BY_BACKUP.value(),
        CopyGeneratedByEnum.BY_REPLICATED.value(), CopyGeneratedByEnum.BY_CASCADED_REPLICATION.value(),
        CopyGeneratedByEnum.BY_REVERSE_REPLICATION.value());

    @Autowired
    public void setJobService(JobService jobService) {
        this.jobService = jobService;
    }

    @Autowired
    public void setCopyRestApi(CopyRestApi copyRestApi) {
        this.copyRestApi = copyRestApi;
    }

    @Autowired
    public void setDmeReplicationRestApi(DmeReplicationRestApi dmeReplicationRestApi) {
        this.dmeReplicationRestApi = dmeReplicationRestApi;
    }

    @Autowired
    public void setProtectedEnvironmentService(ProtectedEnvironmentService protectedEnvironmentService) {
        this.protectedEnvironmentService = protectedEnvironmentService;
    }

    @Autowired
    public void setProviderManager(ProviderManager providerManager) {
        this.providerManager = providerManager;
    }

    @Autowired
    public void setDefaultCopyDeleteInterceptor(DefaultCopyDeleteInterceptor defaultCopyDeleteInterceptor) {
        this.defaultCopyDeleteInterceptor = defaultCopyDeleteInterceptor;
    }

    @Autowired
    public void setCopyMapper(CopyMapper copyMapper) {
        this.copyMapper = copyMapper;
    }

    @Override
    public Copy queryCopyFromJobId(String jobId) {
        if (VerifyUtil.isEmpty(jobId)) {
            log.warn("job id is empty");
            return null;
        }
        JobBo jobBo = jobService.queryJob(jobId);
        if (jobBo == null) {
            log.warn("job do not found. job id: {}", jobId);
            return null;
        }
        String copyId = jobBo.getCopyId();
        if (VerifyUtil.isEmpty(copyId)) {
            log.warn("copy id is empty. job id: {}", jobId);
            return null;
        }
        Copy copy = copyRestApi.queryCopyByID(copyId, false);
        if (copy == null) {
            log.warn("copy do not found. job id: {}, copy id: {}", jobId, copyId);
            return null;
        }
        return copy;
    }

    @Override
    public void notifyWhenCopyDeleted(String jobId, Copy copy) {
        JobBo jobBo = jobService.queryJob(jobId);
        if (jobBo == null) {
            log.warn("job do not found. job id: {}", jobId);
            return;
        }
        if (copy == null) {
            log.debug("copy is null. job id: {}", jobId);
            return;
        }
        // 是否为备份副本，复制
        if (!notifyCopyDeleteTypeList.contains(copy.getGeneratedBy())) {
            log.debug("copy generate by is :{}, job id: {}, copy id: {}", copy.getGeneratedBy(), jobId, copy.getUuid());
            return;
        }
    }

    @Override
    public TaskResource buildTaskResource(Copy copy) {
        final String resourceProperties = copy.getResourceProperties();
        final JSONObject targetJsonObject = JSONObjectCovert.covertLowerUnderscoreKeyToLowerCamel(
            JSONObject.fromObject(resourceProperties));
        final ProtectedResource protectedResource = targetJsonObject.toBean(ProtectedResource.class);
        TaskResource taskResource = new TaskResource();
        BeanUtils.copyProperties(protectedResource, taskResource);
        return taskResource;
    }

    @Override
    public TaskEnvironment buildTaskEnvironment(String rootUuid) {
        ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(rootUuid);
        if (environment != null) {
            return covertToTaskEnvironment(environment);
        }
        return new TaskEnvironment();
    }

    private TaskEnvironment covertToTaskEnvironment(ProtectedEnvironment protectedEnvironment) {
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(protectedEnvironment, taskEnvironment);
        return taskEnvironment;
    }

    /**
     * 调用复制微服务删除副本
     *
     * @param jobBo 任务信息
     * @param copy 副本信息
     */
    private void removeDmeReplication(JobBo jobBo, Copy copy) {
        RemoveCopyRequest removeCopyRequest = new RemoveCopyRequest();
        removeCopyRequest.setCopyId(copy.getUuid());
        removeCopyRequest.setResourceId(copy.getResourceId());
        removeCopyRequest.setJobType(jobBo.getType());
        removeCopyRequest.setCopyType(
            CopyGeneratedByEnum.BY_BACKUP.value().equalsIgnoreCase(copy.getGeneratedBy()) ? 0 : 1);
        List<ReplicatedCopies> replicatedCopiesList = copyRestApi.queryReplicationCopies(copy.getUuid());
        List<String> esnList = replicatedCopiesList.stream()
            .map(replicatedCopies -> replicatedCopies.getEsn())
            .collect(Collectors.toList());
        log.info("notifyWhenCopyDeleted,jobId:{},esnSize:{}", jobBo.getJobId(), esnList.size());
        for (String ens : esnList) {
            removeCopyRequest.setTargetEsn(ens);
            log.info("call dme delete copy api. job id: {}, copy id: {}, targetEsn: {}, copyType: {}", jobBo.getJobId(),
                removeCopyRequest.getCopyId(), removeCopyRequest.getTargetEsn(), removeCopyRequest.getCopyType());
            dmeReplicationRestApi.deleteCopy(removeCopyRequest);
            log.info("call dme delete copy api success. job id: {}", jobBo.getJobId());
        }
    }

    @Override
    public List<String> getAssociatedCopies(String copyId) {
        Copy copy = copyRestApi.queryCopyByID(copyId, false);
        if (copy == null) {
            log.warn("Copy({}) can not be found when get associated copies.", copyId);
            return Collections.emptyList();
        }

        CopyDeleteInterceptor interceptor = providerManager.findProviderOrDefault(CopyDeleteInterceptor.class,
            copy.getResourceSubType(), defaultCopyDeleteInterceptor);
        return interceptor.getAssociatedCopy(copyId);
    }

    @Override
    public PageListResponse<CopySummaryResource> queryCopySummaryResource(int pageSize, int pageNo, String[] orders,
        CopySummaryResourceCondition condition) {
        CopySummaryResourceQuery summaryResourceQuery = getCopySummaryResourceQuery(pageSize, pageNo, orders,
            condition);
        PageListResponse<CopySummaryResource> pageListResponse = new PageListResponse<>();
        int count = copyMapper.selectCopySummaryResourceCount(summaryResourceQuery);
        pageListResponse.setTotalCount(count);
        if (count == 0) {
            return pageListResponse;
        }
        pageListResponse.setRecords(copyMapper.selectCopySummaryResourceList(summaryResourceQuery));
        return pageListResponse;
    }

    private CopySummaryResourceQuery getCopySummaryResourceQuery(int pageSize, int pageNo, String[] orders,
        CopySummaryResourceCondition condition) {
        CopySummaryResourceQuery copySummaryResourceQuery = new CopySummaryResourceQuery();
        copySummaryResourceQuery.setPageNo(pageNo);
        copySummaryResourceQuery.setPageSize(pageSize);
        copySummaryResourceQuery.setCondition(condition);
        copySummaryResourceQuery.setOrders(getValidOrders(orders));
        return copySummaryResourceQuery;
    }

    private List<String> getValidOrders(String[] orders) {
        List<String> validOrders = new ArrayList<>();
        if (VerifyUtil.isEmpty(orders)) {
            return validOrders;
        }
        for (String order : orders) {
            if (VerifyUtil.isEmpty(order)) {
                continue;
            }
            getDbOrderField(order, ORDER_INCREASE_PREFIX).ifPresent(validOrders::add);
            getDbOrderField(order, ORDER_DECREASE_PREFIX).ifPresent(validOrders::add);
        }
        return validOrders;
    }

    private Optional<String> getDbOrderField(String order, String orderPrefix) {
        if (order.startsWith(orderPrefix)) {
            String dbField = ORDER_FIELD_MAP.get(order.substring(1));
            if (dbField != null) {
                return Optional.of(orderPrefix + dbField);
            }
        }
        return Optional.empty();
    }

    @Override
    public boolean checkSanCopy(String copyId) {
        Copy copy = copyRestApi.queryCopyByID(copyId);
        if (VerifyUtil.isEmpty(copy)) {
            return false;
        }
        return CopyUtil.checkIsSanCopy(copy);
    }

    @Override
    public List<CopySummaryCount> queryCopyCount(String domainId) {
        List<CopySummaryCount> copySummaryCounts = copyMapper.queryCopyCount(domainId);
        if (copySummaryCounts == null) {
            copySummaryCounts = Collections.emptyList();
        }
        return copySummaryCounts;
    }

    @Override
    public void updateCopyStatus(List<String> copyIdList, String status) {
        log.info("Update copy status, copyIsList: {}, status: {}.", StringUtils.join(copyIdList, ","), status);
        copyMapper.updateCopyStatus(copyIdList, status);
    }

    @Override
    public CopiesEntity queryCopyById(String copyId) {
        QueryWrapper<CopiesEntity> wrapper = new QueryWrapper<>();
        wrapper.eq("uuid", copyId);
        return copyMapper.selectOne(wrapper);
    }

    @Override
    public Long queryCopyCounts(String resourceId, String esn, String storageUnitId, List<Integer> backupTypes) {
        QueryWrapper<CopiesEntity> wrapper = new QueryWrapper<>();
        if (!VerifyUtil.isEmpty(resourceId)) {
            wrapper.eq("resource_id", resourceId);
        }
        if (!VerifyUtil.isEmpty(esn)) {
            wrapper.eq("device_esn", esn);
        }
        if (!VerifyUtil.isEmpty(storageUnitId)) {
            wrapper.eq("storage_unit_id", storageUnitId);
        }
        if (!VerifyUtil.isEmpty(backupTypes)) {
            wrapper.in("backup_type", backupTypes);
        }
        return copyMapper.selectCount(wrapper);
    }
}
