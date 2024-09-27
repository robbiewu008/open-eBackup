/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 */

package openbackup.data.access.framework.copy.mng.service;

import openbackup.data.access.client.sdk.api.framework.dme.replicate.DmeReplicationRestApi;
import openbackup.data.access.framework.copy.mng.service.impl.CopyManagerServiceImpl;
import openbackup.data.access.framework.core.dao.CopyMapper;
import openbackup.data.access.framework.core.model.CopySummaryCount;
import openbackup.data.access.framework.core.model.CopySummaryResource;
import openbackup.data.access.framework.core.model.CopySummaryResourceQuery;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.List;

/**
 * CopyManagerServiceTest
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-07
 */
public class CopyManagerServiceTest {
    private CopyManagerServiceImpl copyManagerService;

    private ClusterInternalApi clusterInternalApi;

    private DmeReplicationRestApi dmeReplicationRestApi;
    private JobService jobService;
    private CopyRestApi copyRestApi;
    private CopyMapper copyMapper;
    @Before
    public void before() {
        copyManagerService = new CopyManagerServiceImpl();
        clusterInternalApi = Mockito.mock(ClusterInternalApi.class);
        jobService = Mockito.mock(JobService.class);
        copyRestApi = Mockito.mock(CopyRestApi.class);
        copyMapper = Mockito.mock(CopyMapper.class);
        dmeReplicationRestApi = Mockito.mock(DmeReplicationRestApi.class);
        copyManagerService.setDmeReplicationRestApi(dmeReplicationRestApi);
        copyManagerService.setJobService(jobService);
        copyManagerService.setCopyRestApi(copyRestApi);
        copyManagerService.setCopyMapper(copyMapper);
        TargetClusterVo targetClusterVo = new TargetClusterVo();
        targetClusterVo.setEsn("esn");
        Mockito.when(clusterInternalApi.queryTargetClusterDetailsByClusterId(Mockito.anyInt()))
            .thenReturn(targetClusterVo);

    }

    @Test
    public void notify_when_copy_deleted_success() {
        Copy copy = new Copy();
        copy.setUuid("uuid");
        copy.setResourceId("resourceId");
        copy.setSlaProperties("{\n" + "  \"uuid\": \"aeb66dfc-8633-4edc-a0f1-d2ec9cdb3764\",\n" + "  \"type\": 1,\n"
            + "  \"policy_list\": [\n" + "    {\n" + "      \"uuid\": \"1c8187e8-4bbc-49e9-9035-4d9755d7aa1d\",\n"
            + "      \"name\": \"全量01\",\n" + "      \"type\": \"backup\",\n" + "      \"action\": \"full\",\n"
            + "      \"ext_parameters\": {\n" + "        \"qos_id\": \"\",\n"
            + "        \"source_deduplication\": false,\n" + "        \"alarm_after_failure\": false,\n"
            + "        \"auto_retry\": true,\n" + "        \"auto_retry_times\": 3,\n"
            + "        \"auto_retry_wait_minutes\": 5\n" + "      },\n" + "      \"active\": true,\n"
            + "      \"is_active\": true\n" + "    },\n" + "    {\n"
            + "      \"uuid\": \"6fdc8b15-c40c-4fd8-ae87-0b8b404beb58\",\n" + "      \"name\": \"策略0\",\n"
            + "      \"type\": \"replication\",\n" + "      \"action\": \"replication\",\n"
            + "      \"ext_parameters\": {\n" + "        \"qos_id\": \"\",\n"
            + "        \"external_system_id\": \"2\",\n" + "        \"link_deduplication\": false,\n"
            + "        \"link_compression\": false,\n" + "        \"alarm_after_failure\": false,\n"
            + "        \"start_replicate_time\": \"2022-10-06 21:09:12\"\n" + "      },\n" + "      \"active\": true,\n"
            + "      \"is_active\": true\n" + "    }\n" + "  ]\n" + "}");
        copy.setProperties("{\n" + "  \"format\":1\n" + "}");
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        JobBo jobBo = new JobBo();
        jobBo.setType("COPY_DELETE");
        Mockito.when(jobService.queryJob(Mockito.anyString()))
                .thenReturn(jobBo);
        copyManagerService.notifyWhenCopyDeleted("11", copy);
        Assert.assertNotNull(copy);
    }

    /**
     * 用例场景：查询数量为0的副本资源
     * 前提条件：无
     * 检查点：当数量为0时，返回值recodes为空，totalCount为0
     */
    @Test
    public void query_copy_summary_resource_count_zero() {
        CopySummaryResourceQuery copySummaryResourceQuery = new CopySummaryResourceQuery();
        copySummaryResourceQuery.setPageNo(0);
        copySummaryResourceQuery.setPageSize(10);
        copySummaryResourceQuery.setCondition(null);
        copySummaryResourceQuery.setOrders(new ArrayList<>());
        Mockito.when(copyMapper.selectCopySummaryResourceCount(Mockito.eq(copySummaryResourceQuery))).thenReturn(0);
        PageListResponse<CopySummaryResource> response = copyManagerService.queryCopySummaryResource(10, 0, null, null);
        Assert.assertEquals(0, response.getTotalCount());
        Assert.assertTrue(VerifyUtil.isEmpty(response.getRecords()));
    }

    /**
     * 用例场景：查询副本资源列表，使用正确的排序字段
     * 前提条件：无
     * 检查点：[+-]copyCount, [+-]displayTimestamp为有效的排序字段，其余为无效
     */
    @Test
    public void query_order_should_correct_when_query_copy_summary_resource() {
        CopySummaryResourceQuery copySummaryResourceQuery = new CopySummaryResourceQuery();
        copySummaryResourceQuery.setPageNo(0);
        copySummaryResourceQuery.setPageSize(10);
        copySummaryResourceQuery.setCondition(null);
        copySummaryResourceQuery.setOrders(new ArrayList<>());
        Mockito.when(copyMapper.selectCopySummaryResourceCount(Mockito.eq(copySummaryResourceQuery))).thenReturn(1);

        CopySummaryResourceQuery copySummaryResourceQuery2 = new CopySummaryResourceQuery();
        copySummaryResourceQuery2.setPageNo(0);
        copySummaryResourceQuery2.setPageSize(10);
        copySummaryResourceQuery2.setCondition(null);
        List<String> orders = new ArrayList<>();
        copySummaryResourceQuery2.setOrders(orders);
        orders.add("+copy_count");
        orders.add("-display_timestamp");
        Mockito.when(copyMapper.selectCopySummaryResourceCount(Mockito.eq(copySummaryResourceQuery2))).thenReturn(2);
        // 不传orders
        PageListResponse<CopySummaryResource> response = copyManagerService.queryCopySummaryResource(10, 0, null, null);
        Assert.assertEquals(1, response.getTotalCount());
        // 传的orders不在规定中
        PageListResponse<CopySummaryResource> response2 = copyManagerService.queryCopySummaryResource(10, 0,
            new String[] {"copyCount", "-football", "+sky", "", null}, null);
        Assert.assertEquals(1, response2.getTotalCount());
        // 传的orders在规定中
        PageListResponse<CopySummaryResource> response3 = copyManagerService.queryCopySummaryResource(10, 0,
            new String[] {"+copyCount", "-displayTimestamp"}, null);
        Assert.assertEquals(2, response3.getTotalCount());
    }

    /**
     * 用例场景：查询副本统计信息
     * 前提条件：无
     * 检查点：查询副本统计信息成功
     */
    @Test
    public void query_copy_count_success() {
        List<CopySummaryCount> copySummaryCounts = new ArrayList<>();
        copySummaryCounts.add(new CopySummaryCount());
        Mockito.when(copyMapper.queryCopyCount(null)).thenReturn(copySummaryCounts).thenReturn(null);

        Assert.assertEquals(copyManagerService.queryCopyCount(null).size(), 1);
        Assert.assertEquals(copyManagerService.queryCopyCount(null).size(), 0);
    }
}
