/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.kingbase.protection.access.provider.copy;

import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.dme.AvailableTimeRanges;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * {@link KingBaseCopyDeleteInterceptor} 测试类
 *
 * @author wwx1013713
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-03-17
 */
public class KingBaseCopyDeleteInterceptorTest {
    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final CopyService copyService = PowerMockito.mock(CopyService.class);

    private final KingBaseCopyDeleteInterceptor interceptor = new KingBaseCopyDeleteInterceptor(copyRestApi,
        resourceService, copyService);

    /**
     * 用例场景：Kingbase副本删除是否填充agent
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：返回false，不填充agent
     */
    @Test
    public void shouldSupplyAgent_success() {
        DeleteCopyTask task = new DeleteCopyTask();
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        Assert.assertFalse(interceptor.shouldSupplyAgent(task, copyInfoBo));
    }

    /**
     * 用例场景：传入的资源子类型匹配则使用当前删除副本Provider
     * 前置条件：资源子类型是KingBaseInstance或者KingBaseClusterInstance
     * 检查点：返回true
     */
    @Test
    public void applicable_success() {
        List<String> resourceSubTypes = Arrays.asList(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType(),
            ResourceSubTypeEnum.KING_BASE_CLUSTER_INSTANCE.getType());
        for (String tmpResourceSubType : resourceSubTypes) {
            Assert.assertTrue(interceptor.applicable(tmpResourceSubType));
        }
    }

    /**
     * 用例场景：传入的资源子类型不匹配则不使用当前删除副本Provider
     * 前置条件：资源子类型不是KingBaseInstance或者KingBaseClusterInstance
     * 检查点：返回false
     */
    @Test
    public void should_return_false_if_resourceSubType_is_matched_when_applicable() {
        List<String> resourceSubTypes = Arrays.asList(ResourceSubTypeEnum.KING_BASE_CLUSTER.getType(),
            ResourceSubTypeEnum.KING_BASE.getType(), ResourceSubTypeEnum.POSTGRE_INSTANCE.getType());
        for (String tmpResourceSubType : resourceSubTypes) {
            Assert.assertFalse(interceptor.applicable(tmpResourceSubType));
        }
    }

    /**
     * 用例场景：收集删除全量副本时需要关联删除的副本ID列表
     * 前置条件：要删除的全量副本没有关联的日志副本
     * 检查点：返回空
     */
    @Test
    public void getCopiesCopyTypeIsFull_success_if_delFull_and_noAssociatedLog() {
        String copyId = "0c980760-9eb0-4280-aa54-08367686bcec";
        Copy thisCopy = new Copy();
        thisCopy.setUuid(copyId);
        thisCopy.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        thisCopy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        List<String> needDelCopyIds = interceptor.getCopiesCopyTypeIsFull(new ArrayList<>(), thisCopy, null);
        Assert.assertEquals(needDelCopyIds.size(), 0);
    }

    private Copy buildFakeFullCopy() {
        Copy fullCopy = new Copy();
        fullCopy.setUuid("0c980760-9eb0-4280-aa54-08367686bcec");
        fullCopy.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        fullCopy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        fullCopy.setProperties("{\"backupTime\":1678972640}");
        fullCopy.setGn(10);
        fullCopy.setResourceId("81a8d9af031a4e498ced81e4f59f2156");
        return fullCopy;
    }

    private Copy buildFakeLogCopy() {
        Copy logCopy = new Copy();
        logCopy.setUuid("dcd55d0e-0bb5-4643-aed7-32b183c676ea");
        logCopy.setBackupType(BackupTypeConstants.LOG.getAbBackupType());
        logCopy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        logCopy.setGn(11);
        logCopy.setResourceId("81a8d9af031a4e498ced81e4f59f2156");
        return logCopy;
    }

    /**
     * 用例场景：收集删除全量副本时需要关联删除的副本ID列表
     * 前置条件：要删除的全量副本有关联的日志副本，日志副本开始时间等于全量副本备份时间
     * 检查点：返回日志副本ID列表
     */
    @Test
    public void getCopiesCopyTypeIsFull_success_if_delFull_and_associatedLog_and_logStartTimeEqualFullBakTime() {
        Copy thisCopy = buildFakeFullCopy();
        Copy firstLogCopy = buildFakeLogCopy();
        firstLogCopy.setProperties("{\"beginTime\":1678972640,\"endTime\":1678974888,\"backupTime\":1678974888}");
        List<String> needDelCopyIds = interceptor.getCopiesCopyTypeIsFull(Collections.singletonList(firstLogCopy),
            thisCopy, null);
        Assert.assertEquals(needDelCopyIds.get(0), "dcd55d0e-0bb5-4643-aed7-32b183c676ea");
    }

    /**
     * 用例场景：收集删除全量副本时需要关联删除的副本ID列表
     * 前置条件：要删除的全量副本有关联的日志副本，日志副本开始时间小于全量副本备份时间，日志副本开始时间关联副本不存在
     * 检查点：返回日志副本ID列表
     */
    @Test
    public void getCopiesCopyTypeIsFull_success_if_delFullAssociateLog_logStartTLessFullBakT_beginTCopyNotExists() {
        Copy thisCopy = buildFakeFullCopy();
        Copy firstLogCopy = buildFakeLogCopy();
        firstLogCopy.setProperties("{\"beginTime\":1678972458,\"endTime\":1678974888,\"backupTime\":1678974888}");
        PageListResponse<AvailableTimeRanges> pageListResponse = new PageListResponse<>(0, Collections.emptyList());
        PowerMockito.when(copyService.listAvailableTimeRanges(anyString(), anyLong(), anyLong(), anyInt(), anyInt()))
            .thenReturn(pageListResponse);
        List<String> needDelCopyIds = interceptor.getCopiesCopyTypeIsFull(Collections.singletonList(firstLogCopy),
            thisCopy, null);
        Assert.assertEquals(needDelCopyIds.get(0), "dcd55d0e-0bb5-4643-aed7-32b183c676ea");
    }

    /**
     * 用例场景：收集删除全量副本时需要关联删除的副本ID列表
     * 前置条件：要删除的全量副本有关联的日志副本，日志副本开始时间小于全量副本备份时间，日志副本开始时间关联副本存在
     * 检查点：返回空列表
     */
    @Test
    public void getCopiesCopyTypeIsFull_success_if_delFullAssociateLog_logStartTLessFullBakT_beginTCopyExists() {
        Copy thisCopy = buildFakeFullCopy();
        Copy firstLogCopy = buildFakeLogCopy();
        firstLogCopy.setProperties("{\"beginTime\":1678972458,\"endTime\":1678974888,\"backupTime\":1678974888}");
        AvailableTimeRanges timeRanges = new AvailableTimeRanges();
        timeRanges.setCopyId("dcd55d0e-0bb5-4643-aed7-32b183c676ea");
        timeRanges.setStartTime(1678972458);
        timeRanges.setEndTime(1678972459);
        PageListResponse<AvailableTimeRanges> pageListResponse = new PageListResponse<>(1,
            Collections.singletonList(timeRanges));
        PowerMockito.when(copyService.listAvailableTimeRanges(anyString(), anyLong(), anyLong(), anyInt(), anyInt()))
            .thenReturn(pageListResponse);
        List<String> needDelCopyIds = interceptor.getCopiesCopyTypeIsFull(Collections.singletonList(firstLogCopy),
            thisCopy, null);
        Assert.assertEquals(needDelCopyIds.size(), 0);
    }
}
