/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.access.framework.resource.service.impl;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.persistence.dao.ProtectedResourceExtendInfoMapper;
import openbackup.access.framework.resource.persistence.model.ProtectedResourceExtendInfoPo;
import openbackup.access.framework.resource.service.impl.ResourceExtendInfoServiceImpl;
import openbackup.data.protection.access.provider.sdk.resource.model.ProtectedResourceExtendInfo;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.transaction.support.TransactionTemplate;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * ResourceExtendInfoServiceImpl test
 *
 * @author w30044259
 * @since 2023-07-29
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@ContextConfiguration(classes = {ResourceExtendInfoServiceImpl.class})
public class ResourceExtendInfoServiceImplTest {
    @MockBean
    private ProtectedResourceExtendInfoMapper protectedResourceExtendInfoMapper;

    @Autowired
    private ResourceExtendInfoServiceImpl resourceExtendInfoService;

    @MockBean
    private TransactionTemplate transactionTemplate;

    @Before
    public void before(){
        Mockito.when(protectedResourceExtendInfoMapper.delete(any())).thenReturn(1);
    }

    /**
     * 用例名称：删除资源扩展表指定key的数据，正常删除
     * 前置条件：对数据库打桩，表中已存在待删除的数据。
     * check点：再次查询，无已经被删除的数据。
     */
    @Test
    public void test_deleteByKeys_successfully(){
        resourceExtendInfoService.deleteByKeys("id","key");
        Mockito.verify(protectedResourceExtendInfoMapper).delete(any());
    }

    /**
     * 用例名称：删除资源扩展表指定key的数据，删除失败
     * 前置条件：打桩，数据库访问异常。
     * check点：再次查询，被删除的数据依然存在。0.
     */
    @Test(expected = RuntimeException.class)
    public void test_deleteByKeys_fail(){
        Mockito.doThrow(new RuntimeException("failed to browser database")).when(protectedResourceExtendInfoMapper.delete(any()));
        resourceExtendInfoService.deleteByKeys("id","key");
    }


    /**
     * 用例名称：保存指定资源的数据，
     * 前置条件：对数据库打桩。
     * check点：保存成功。
     */
    @Test
    public void test_saveOrUpdateExtendInfo(){
        String connectionResult="{\n" +
                "    \"uuid\": {\n" +
                "        \"end_point\": \"127.0.0.1\",\n" +
                "        \"link_status\": 0,\n" +
                "        \"cluster_name\": \"name\"\n" +
                "    },\n" +
                "    \"esn\": {\n" +
                "        \"end_point\": \"127.0.0.1\",\n" +
                "        \"link_status\": 1,\n" +
                "        \"cluster_name\": \"name\"\n" +
                "    }\n" +
                "}";
        Map<String,String> map=new HashMap<>();
        map.put("connection_result",connectionResult);
        map.put("key",connectionResult);
        resourceExtendInfoService.saveOrUpdateExtendInfo("key",map);
        ProtectedResourceExtendInfoPo resourceExtendInfoPo = new ProtectedResourceExtendInfoPo();
        resourceExtendInfoPo.setUuid("UUID");
        resourceExtendInfoPo.setKey("connection_result");
        resourceExtendInfoPo.setValue(connectionResult);
        Mockito.when(protectedResourceExtendInfoMapper.selectOne(any())).thenReturn(null).thenReturn(resourceExtendInfoPo);
        resourceExtendInfoService.saveOrUpdateExtendInfo("envId",map);
        Assert.assertTrue(true);
    }

    /**
     * 用例名称：查询指定资源的数据，
     * 前置条件：对数据库打桩，表中已存在待查询的数据。
     * check点：查询成功。
     */
    @Test
    public void test_queryExtendInfo(){
        ProtectedResourceExtendInfoPo resourceExtendInfoPo = new ProtectedResourceExtendInfoPo();
        resourceExtendInfoPo.setUuid("UUID");
        resourceExtendInfoPo.setKey("connection_result");
        String connectionResult="{\n" +
                "    \"uuid\": {\n" +
                "        \"end_point\": \"127.0.0.1\",\n" +
                "        \"link_status\": 0,\n" +
                "        \"cluster_name\": \"name\"\n" +
                "    },\n" +
                "    \"esn\": {\n" +
                "        \"end_point\": \"127.0.0.1\",\n" +
                "        \"link_status\": 1,\n" +
                "        \"cluster_name\": \"name\"\n" +
                "    }\n" +
                "}";
        resourceExtendInfoPo.setValue(connectionResult);
        Mockito.when(protectedResourceExtendInfoMapper.selectList(any())).thenReturn(Arrays.asList(resourceExtendInfoPo)).thenReturn(null);
        Map<String, String> map = resourceExtendInfoService.queryExtendInfo("envId", "connection_result");
        Assert.assertFalse(map.isEmpty());
        map = resourceExtendInfoService.queryExtendInfo("envId","connection_result");
        Assert.assertTrue(map.isEmpty());
    }

    /**
     * 用例名称：查询指定资源的数据，
     * 前置条件：对数据库打桩，表中已存在待查询的数据。
     * check点：查询agent成功，返回值通过resource id分组聚合。
     */
    @Test
    public void test_queryExtendInfoByResourceIds() {
        ProtectedResourceExtendInfoPo extendInfo1 = new ProtectedResourceExtendInfoPo();
        extendInfo1.setResourceId("1");

        ProtectedResourceExtendInfoPo extendInfo2 = new ProtectedResourceExtendInfoPo();
        extendInfo2.setResourceId("1");

        Mockito.when(protectedResourceExtendInfoMapper.selectList(any()))
                .thenReturn(Arrays.asList(extendInfo1, extendInfo2));
        Map<String, List<ProtectedResourceExtendInfo>> extendInfoMap = resourceExtendInfoService
                .queryExtendInfoByResourceIds(new ArrayList<>(Collections.singletonList("1")));
        Assert.assertEquals(extendInfoMap.get("1").size(), 2);
    }
}
