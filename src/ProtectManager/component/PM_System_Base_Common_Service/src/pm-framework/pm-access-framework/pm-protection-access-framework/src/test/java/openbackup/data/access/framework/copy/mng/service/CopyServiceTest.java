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
package openbackup.data.access.framework.copy.mng.service;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;

import com.huawei.oceanprotect.base.cluster.sdk.dto.ClusterRequestInfo;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;

import openbackup.data.access.client.sdk.api.framework.dme.AvailableTimeRanges;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.copy.mng.service.impl.CopyServiceImpl;
import openbackup.data.access.framework.core.dao.CopiesProtectionMapper;
import openbackup.data.access.framework.core.dao.CopyMapper;
import openbackup.data.access.framework.core.entity.CopiesEntity;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.copy.CopyCommonInterceptor;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.cluster.TargetClusterRestApi;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.dee.DeeBaseParseRest;
import openbackup.system.base.sdk.dee.DeeInternalCopyRest;
import openbackup.system.base.sdk.dee.model.FineGrainedRestore;
import openbackup.system.base.sdk.dee.model.RestoreFilesResponse;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * 副本相关的测试类
 *
 */

@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = CopyServiceImpl.class)
@PrepareForTest( {TokenBo.class})
public class CopyServiceTest {
    private static final String RESOURCE_ID = "83445bf0-f601-4509-b6c1-66634318206a";

    private static final String COPY_ID = "83445bf0-f601-4509-sdfs-66634318206a";

    private static final long START_TIME = 33333L;

    private static final long END_TIME = 55555L;

    private static final long LARGE_FILE_SIZE = 4588558576L;

    @Autowired
    private CopyService copyService;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private DeeInternalCopyRest deeInternalCopyRest;

    @MockBean
    private DmeUnifiedRestApi dmeUnifiedRestApi;

    @MockBean
    private DeeBaseParseRest deeBaseParseRest;

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    private MemberClusterService memberClusterService;

    @MockBean
    @Qualifier("targetClusterApiWithDmaProxyManagePort")
    private TargetClusterRestApi targetApi;

    @MockBean
    private CopyMapper copyMapper;

    @MockBean
    private CopiesProtectionMapper copiesProtectionMapper;

    @MockBean
    private StorageUnitService storageUnitService;

    @MockBean
    private SessionService sessionService;

    /**
     * 用例名称：查询指定时间范围可用于恢复的时间段<br/>
     * 前置条件：副本存在<br/>
     * check点：数据正常返回<br/>
     */
    @Test
    public void list_available_time_ranges_success() {
        PowerMockito.when(
                dmeUnifiedRestApi.listAvailableTimeRanges(anyString(), anyLong(), anyLong(), anyInt(), anyInt()))
            .thenReturn(getAvailableTimeRangesPageListResponse());
        PageListResponse<AvailableTimeRanges> response = copyService.listAvailableTimeRanges(RESOURCE_ID, START_TIME,
            END_TIME, 50, 0);
        Assert.assertEquals(1, response.getTotalCount());
        Assert.assertNotNull(response.getRecords());
        Assert.assertEquals(START_TIME, response.getRecords().get(0).getStartTime());
    }

    /**
     * 用例名称：查询指定时间范围可用于恢复的时间段<br/>
     * 前置条件：副本存在<br/>
     * check点：校验查询指定时间范围如果开始时间时间大于结束时间抛出错误<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_if_startTime_longer_than_endTime_list_available_time_ranges() {
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> copyService.listAvailableTimeRanges(RESOURCE_ID, END_TIME, START_TIME, 50, 0));
        Assert.assertTrue(legoCheckedException.getMessage().contains("startTime or endTime is illegal"));
    }

    private PageListResponse<AvailableTimeRanges> getAvailableTimeRangesPageListResponse() {
        PageListResponse<AvailableTimeRanges> availableTimeRanges = new PageListResponse<>();
        AvailableTimeRanges timeRanges = new AvailableTimeRanges();
        timeRanges.setCopyId(COPY_ID);
        timeRanges.setStartTime(START_TIME);
        timeRanges.setEndTime(END_TIME);
        availableTimeRanges.setTotalCount(1);
        availableTimeRanges.setRecords(Collections.singletonList(timeRanges));
        return availableTimeRanges;
    }

    @Test
    public void should_forward_while_copy_in_remote_node() {
        Copy copy = new Copy();
        copy.setUuid("123");
        copy.setDeviceEsn("123");
        ClusterRequestInfo clusterRequestInfo = new ClusterRequestInfo();
        clusterRequestInfo.setToken("123");
        clusterRequestInfo.setIp("1.2.3.4");
        clusterRequestInfo.setPort(22);
        PowerMockito.when(memberClusterService.isNeedForward(anyString())).thenReturn(true);
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        PowerMockito.when(memberClusterService.getClusterRequestInfo(anyString())).thenReturn(clusterRequestInfo);
        copyService.listCopyCatalogs("123", "123", 0, 0, "{}");
        Mockito.verify(targetApi).listCopyCatalogs(any(), anyString(), anyString(), anyMap());
    }

    private FineGrainedRestore mockFile() {
        FineGrainedRestore normalFile = new FineGrainedRestore();
        normalFile.setHasChildren(false);
        normalFile.setModifyTime("2021-12-25 16:33");
        normalFile.setSize(10L);
        normalFile.setPath("/mnt");
        normalFile.setType("file");
        return normalFile;
    }

    /**
     * 用例名称：浏览副本中文件和目录信息<br/>
     * 前置条件：副本存在<br/>
     * check点：数据正常返回<br/>
     */
    @Test
    public void list_copy_catalogs_success() {
        final String copyId1 = "1111";
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.eq(copyId1))).thenReturn(mockCopyExistSnapshots());
        FineGrainedRestore normalFile = mockFile();

        PowerMockito.when(deeInternalCopyRest.listCopyCatalogs(ArgumentMatchers.any()))
            .thenReturn(mockRestoreFilesResponse(normalFile));
        CopyCommonInterceptor mock = Mockito.mock(CopyCommonInterceptor.class);
        PowerMockito.when(providerManager.findProvider(any(), any(), any())).thenReturn(mock);
        PageListResponse<FineGrainedRestore> response = copyService.listCopyCatalogs(copyId1, "/mnt", 10, 0, "{}");
        AssertResponse(response);
        final String copyId2 = "2222";
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.eq(copyId2))).thenReturn(mockCopy());
        PowerMockito.when(deeInternalCopyRest.listCopyCatalogs(ArgumentMatchers.any()))
            .thenReturn(mockRestoreFilesResponse(normalFile));

        PageListResponse<FineGrainedRestore> response1 = copyService.listCopyCatalogs(copyId2, "/mnt2", 10, 0, "{}");
        AssertResponse(response);
    }

    /**
     * 用例名称：浏览副本中文件和目录信息, 副本中文件大小超大，超过INTEGER范围<br/>
     * 前置条件：副本存在, 副本中文件大小超大，超过INTEGER范围<br/>
     * check点：数据正常返回<br/>
     */
    @Test
    public void list_copy_catalogs_success_when_response_with_large_size_file() {
        final String copyId1 = "1111";
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.eq(copyId1))).thenReturn(mockCopyExistSnapshots());
        FineGrainedRestore largeFile = mockFile();
        largeFile.setSize(LARGE_FILE_SIZE);
        PowerMockito.when(deeInternalCopyRest.listCopyCatalogs(ArgumentMatchers.any()))
            .thenReturn(mockRestoreFilesResponse(largeFile));

        PageListResponse<FineGrainedRestore> response = copyService.listCopyCatalogs(copyId1, "/mnt", 10, 0, "{}");
        Assert.assertEquals((long) response.getRecords().get(0).getSize(), LARGE_FILE_SIZE);
    }

    /**
     * 用例名称：下载副本中的文件<br/>
     * 前置条件：副本存在<br/>
     * check点：参数组装正确，数据正常返回<br/>
     */
    @Test
    public void down_load_files_success() {
        String copyId1 = "down_copyId_test_11";
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.any())).thenReturn(mockCopy());
        String requestId = copyService.downloadFiles(copyId1, Arrays.asList("/down2", "down1"), "1111");
        Assert.assertNotNull(requestId);
    }

    private void AssertResponse(PageListResponse<FineGrainedRestore> response) {
        Assert.assertNotNull(response);
        Assert.assertEquals(response.getTotalCount(), 10);
        Assert.assertEquals(response.getRecords().size(), 1);
        Assert.assertEquals(response.getRecords().get(0).getHasChildren(), false);
    }

    // mok dee返回副本中文件和目录信息
    private RestoreFilesResponse mockRestoreFilesResponse(FineGrainedRestore file) {
        RestoreFilesResponse result = new RestoreFilesResponse();
        result.setTotal(10);
        result.setItems(Collections.singletonList(file));
        return result;
    }

    private Copy mockCopyExistSnapshots() {
        Copy copy = new Copy();
        copy.setResourceSubType("NasFileSystem");
        JSONObject properties = new JSONObject();
        properties.put("isAggregation", false);
        String json = "[{\"id\":\"7@FilesystemSnapshot2201111308220\",\"parentName\":\"xxx\"}]";
        properties.put("snapshots", json);
        copy.setProperties(properties.toString());
        return copy;
    }

    private Copy mockCopy() {
        Copy copy = new Copy();
        copy.setResourceSubType("NasFileSystem");
        JSONObject properties = new JSONObject();
        JSONArray array = new JSONArray();
        properties.put("isAggregation", false);
        properties.put("snapshots", array);
        copy.setProperties(properties.toString());
        return copy;
    }

    @Test
    public void list_copy_resource_summary() {
        PowerMockito.mockStatic(TokenBo.class);
        TokenBo tokenBo = new TokenBo();
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setName("testUserName");
        userBo.setRoles(new ArrayList<>());
        tokenBo.setUser(userBo);
        PowerMockito.when(TokenBo.get()).thenReturn(tokenBo);
        String condition = "{\"resourceSubType\":[\"Fileset\"]}";
        List<CopiesEntity> copiesEntityList = new ArrayList<>();
        CopiesEntity copiesEntity = CopiesEntity.builder().build();
        copiesEntity.setResourceId("a1");
        copiesEntity.setBackupType(1);
        copiesEntityList.add(copiesEntity);
        copyMapper = PowerMockito.mock(CopyMapper.class);
        PowerMockito.when(copyMapper.selectList(any())).thenReturn(copiesEntityList);
        copyService.listCopyResourceSummary(0, 20, condition, null);
    }

    @Test
    public void test_close_guest_system_success() {
        PowerMockito.doNothing().when(deeBaseParseRest).closeCopyGuestSystem(ArgumentMatchers.any());
        copyService.closeCopyGuestSystem("123");
    }
}
