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
package openbackup.data.access.framework.protection.service.archive;

import openbackup.data.access.framework.protection.common.constants.ArchivePolicyKeyConstant;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveRequest;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.DataMoverCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import com.fasterxml.jackson.databind.JsonNode;

import jodd.util.StringUtil;

import org.springframework.stereotype.Component;

import java.util.Objects;

/**
 * 归档任务服务，用于统一的创建归档job
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/18
 **/
@Component
public class ArchiveJobService {
    private static final Integer MILLISECONDS_TO_SECONDS = 1000;

    private final CopyRestApi copyRestApi;

    private final ArchiveTaskService archiveTaskService;

    private final ResourceSetApi resourceSetApi;

    public ArchiveJobService(CopyRestApi copyRestApi, ArchiveTaskService archiveTaskService,
        ResourceSetApi resourceSetApi) {
        this.copyRestApi = copyRestApi;
        this.archiveTaskService = archiveTaskService;
        this.resourceSetApi = resourceSetApi;
    }

    /**
     * 构造归档任务请求对象
     *
     * @param archiveRequest 归档任务请求
     * @return 创建归档job请求 {@code CreateJobRequest}
     */
    public CreateJobRequest buildJobRequest(ArchiveRequest archiveRequest) {
        Copy copy = copyRestApi.queryCopyByID(archiveRequest.getCopyId());
        if (copy == null) {
            throw new DataMoverCheckedException("no matched service found", CommonErrorCode.ERR_PARAM);
        }
        CreateJobRequest job = new CreateJobRequest();
        job.setSourceId(copy.getResourceId());
        job.setSourceType(archiveRequest.getResourceType());
        job.setSourceSubType(archiveRequest.getResourceSubType());
        job.setUserId(StringUtil.isEmpty(copy.getUserId()) ? "" : copy.getUserId());
        job.setType(JobTypeEnum.ARCHIVE.getValue());
        job.setSourceName(copy.getResourceName());
        job.setSourceLocation(copy.getLocation());
        final StorageRepository archiveRepository = getArchiveRepository(archiveRequest);
        final Endpoint endpoint = archiveRepository.getEndpoint();
        String endpointIp = endpoint == null ? "" : endpoint.getIp();
        String targetLocation = Objects.equals(archiveRepository.getProtocol(), RepositoryProtocolEnum.S3.getProtocol())
            ? archiveRepository.getPath().concat("(").concat(endpointIp).concat(")") : endpointIp;
        job.setTargetLocation(targetLocation);
        job.setTargetName(archiveRepository.getPath());
        job.setCopyTime(Long.parseLong(copy.getTimestamp()) / MILLISECONDS_TO_SECONDS);
        job.setCopyId(copy.getUuid());
        job.setEnableStop(true);
        JSONObject jsonObject = new JSONObject();
        if (StringUtil.isNotBlank(archiveRequest.getSlaName())) {
            jsonObject.set("slaName", archiveRequest.getSlaName());
            JsonNode policyNode = JsonUtil.read(archiveRequest.getPolicy(), JsonNode.class);
            jsonObject.set("slaId", policyNode.get("sla_id"));
            // 任务解析sla id通过sla_id解析
            jsonObject.set("sla_id", policyNode.get("sla_id"));
        }
        jsonObject.set("storageId", archiveRepository.getId());
        jsonObject.set("policy", archiveRequest.getPolicy());
        job.setExtendField(jsonObject);
        job.setDomainIdList(resourceSetApi.getRelatedDomainIdList(archiveRequest.getCopyId()));
        return job;
    }

    private StorageRepository getArchiveRepository(ArchiveRequest archiveRequest) {
        JSONObject extParam = JSONObject.fromObject(archiveRequest.getPolicy())
            .getJSONObject(ArchivePolicyKeyConstant.EXT_PARAMETERS_KEY);
        return archiveTaskService.getRepositoryFromPolicyExtParameters(extParam, false);
    }
}
