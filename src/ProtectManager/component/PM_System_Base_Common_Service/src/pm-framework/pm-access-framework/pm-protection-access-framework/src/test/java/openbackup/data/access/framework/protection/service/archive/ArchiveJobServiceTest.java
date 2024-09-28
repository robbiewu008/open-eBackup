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

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;

import openbackup.data.access.framework.protection.mocks.CopyMocker;
import openbackup.data.access.framework.protection.mocks.SlaMock;

import openbackup.data.protection.access.provider.sdk.archive.ArchiveRequest;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import org.junit.Assert;
import org.junit.Test;

import java.util.UUID;

/**
 **/
public class ArchiveJobServiceTest {
    private final ArchiveTaskService archiveTaskService = mock(ArchiveTaskService.class);
    private final CopyRestApi copyRestApi = mock(CopyRestApi.class);
    private final ResourceSetApi resourceSetApi = mock(ResourceSetApi.class);
    private final ArchiveJobService archiveJobService = new ArchiveJobService(copyRestApi, archiveTaskService,
        resourceSetApi);

    /**
     * 用例名称：验证当给定的参数正确的时候，buildJobRequest函数可以返回CreateJobRequest对象<br/>
     * 前置条件：无<br/>
     * check点：1.CreateJobRequest对象非空 2.字段值与期望一致<br/>
     */
    @Test
    public void should_return_job_request_when_buildJobRequest_given_correct_params() {
        // Given
        final String copyId = UUID.randomUUID().toString();
        final ArchiveRequest archiveRequest = new ArchiveRequest();
        archiveRequest.setPolicy(SlaMock.getArchiveS3Policy());
        archiveRequest.setCopyId(copyId);
        archiveRequest.setResourceSubType(ResourceSubTypeEnum.HDFS_FILESET.getType());
        archiveRequest.setResourceType(ResourceTypeEnum.FILESET.getType());
        archiveRequest.setSlaName("test_sla");
        StorageRepository archiveRepository = new StorageRepository();
        archiveRepository.setPath("/a/b/c");
        archiveRepository.setEndpoint(new Endpoint("1", "6.5.2.1", 9099));
        final Copy copy = CopyMocker.mockHdfsCopy();
        given(archiveTaskService.getRepositoryFromPolicyExtParameters(any(), eq(false))).willReturn(archiveRepository);
        given(copyRestApi.queryCopyByID(any())).willReturn(copy);
        // When
        final CreateJobRequest createJobRequest = archiveJobService.buildJobRequest(archiveRequest);
        // Then
        Assert.assertEquals(copy.getResourceId(), createJobRequest.getSourceId());
        Assert.assertEquals(archiveRequest.getResourceType(), createJobRequest.getSourceType());
        Assert.assertEquals(archiveRequest.getResourceSubType(), createJobRequest.getSourceSubType());
        Assert.assertEquals(JobTypeEnum.ARCHIVE.getValue(), createJobRequest.getType());
        Assert.assertEquals(copy.getUserId(), createJobRequest.getUserId());
        Assert.assertEquals(copy.getResourceName(), createJobRequest.getSourceName());
        Assert.assertEquals(copy.getLocation(), createJobRequest.getSourceLocation());
        Assert.assertEquals("6.5.2.1", createJobRequest.getTargetLocation());
        Assert.assertEquals(copy.getUuid(), createJobRequest.getCopyId());
        Assert.assertEquals(Long.valueOf(Long.parseLong(copy.getTimestamp()) / 1000), createJobRequest.getCopyTime());
        Assert.assertTrue(createJobRequest.isEnableStop());
        Assert.assertEquals("test_sla", createJobRequest.getExtendField().get("slaName"));
    }

    /**
     * 用例名称：返回正确的归档目标位置：桶（ip）<br/>
     * 前置条件：无<br/>
     * check点：返回正确的归档目标位置<br/>
     */
    @Test
    public void should_return_correct_job_location_request_when_buildJobRequest() {
        // Given
        final String copyId = UUID.randomUUID().toString();
        final ArchiveRequest archiveRequest = new ArchiveRequest();
        archiveRequest.setPolicy(SlaMock.getArchiveS3Policy());
        archiveRequest.setCopyId(copyId);
        archiveRequest.setResourceSubType(ResourceSubTypeEnum.HDFS_FILESET.getType());
        archiveRequest.setResourceType(ResourceTypeEnum.FILESET.getType());
        archiveRequest.setSlaName("test_sla");
        StorageRepository archiveRepository = new StorageRepository();
        archiveRepository.setPath("/a/b/c");
        archiveRepository.setEndpoint(new Endpoint("1", "6.5.2.1", 9099));
        archiveRepository.setProtocol(RepositoryProtocolEnum.S3.getProtocol());
        final Copy copy = CopyMocker.mockHdfsCopy();
        given(archiveTaskService.getRepositoryFromPolicyExtParameters(any(), eq(false))).willReturn(archiveRepository);
        given(copyRestApi.queryCopyByID(any())).willReturn(copy);
        // When
        final CreateJobRequest createJobRequest = archiveJobService.buildJobRequest(archiveRequest);
        // Then
        Assert.assertEquals(
            archiveRepository.getPath().concat("(").concat(archiveRepository.getEndpoint().getIp()).concat(")"),
            createJobRequest.getTargetLocation());
    }
}
