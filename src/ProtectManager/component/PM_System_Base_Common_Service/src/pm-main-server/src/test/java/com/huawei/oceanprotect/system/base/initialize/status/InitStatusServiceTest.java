package com.huawei.oceanprotect.system.base.initialize.status;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.common.constants.Constants;
import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigStatus;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.Redisson;
import org.redisson.RedissonMap;

import java.util.ArrayList;

/**
 * InitStatusService测试类
 *
 * @author s30031954
 * @since 2023-02-27
 */
@RunWith(PowerMockRunner.class)
public class InitStatusServiceTest {
    @InjectMocks
    private InitStatusService initStatusService;

    @Mock
    private InitNetworkConfigMapper initNetworkConfigMapper;

    @Mock
    private Redisson redisson;

    /**
     * 用例场景：测试初始化配置项值
     * 前置条件：满足条件
     * 检查点：成功
     */
    @Test
    public void testGetInitConfigStatus() {
        init();

        ConfigStatus initConfigStatus = initStatusService.getInitConfigStatus();
        Assert.assertEquals(initConfigStatus.getStatus(), 1);
    }

    private void init() {
        // 前置条件1
        ArrayList<InitConfigInfo> initConfigInfos1 = new ArrayList<>();
        InitConfigInfo initConfigInfo1 = new InitConfigInfo();
        initConfigInfo1.setInitValue("1");
        initConfigInfos1.add(initConfigInfo1);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(Constants.INIT_ERROR_FLAG))
            .thenReturn(initConfigInfos1);

        // 前置条件2
        ArrayList<InitConfigInfo> initConfigInfos2 = new ArrayList<>();
        InitConfigInfo initConfigInfo2 = new InitConfigInfo();
        initConfigInfo2.setInitValue("1");
        initConfigInfos2.add(initConfigInfo2);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.INIT_PROGRESS_CODE))
            .thenReturn(initConfigInfos2);

        // 前置条件3
        ArrayList<InitConfigInfo> initConfigInfos3 = new ArrayList<>();
        InitConfigInfo initConfigInfo3 = new InitConfigInfo();
        initConfigInfo3.setInitValue("1");
        initConfigInfos3.add(initConfigInfo3);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.INIT_PROGRESS_DESC))
            .thenReturn(initConfigInfos3);

        // 前置条件4
        ArrayList<InitConfigInfo> initConfigInfos4 = new ArrayList<>();
        InitConfigInfo initConfigInfo4 = new InitConfigInfo();
        initConfigInfo4.setInitValue("1");
        initConfigInfos4.add(initConfigInfo4);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.INIT_PROGRESS_RATE))
            .thenReturn(initConfigInfos4);

        // 前置条件5
        ArrayList<InitConfigInfo> initConfigInfos5 = new ArrayList<>();
        InitConfigInfo initConfigInfo5 = new InitConfigInfo();
        initConfigInfo5.setInitValue("[1,10]");
        initConfigInfos5.add(initConfigInfo5);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.INIT_ERROR_CODE_PARAM))
            .thenReturn(initConfigInfos5);
    }

    /**
     * 用例场景：测试SetInitProgressCode
     * 前置条件：NA
     * 检查点：设置过程不报错
     */
    @Test
    public void testSetInitProgressCode() {
        PowerMockito.doNothing().when(initNetworkConfigMapper).updateInitConfig(any());
        initStatusService.setInitProgressCode("code", "123");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：测试SetInitProgressDesc
     * 前置条件：NA
     * 检查点：设置过程不报错
     */
    @Test
    public void testSetInitProgressDesc() {
        PowerMockito.doNothing().when(initNetworkConfigMapper).updateInitConfig(any());
        initStatusService.setInitProgressDesc("code", "123");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：测试SetInitProgressParams
     * 前置条件：NA
     * 检查点：设置过程不报错
     */
    @Test
    public void testSetInitProgressParams() {
        PowerMockito.doNothing().when(initNetworkConfigMapper).updateInitConfig(any());
        initStatusService.setInitProgressParams(new ArrayList<>(), "123");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：测试SetInitProgressRate
     * 前置条件：NA
     * 检查点：设置过程不报错
     */
    @Test
    public void testSetInitProgressRate() {
        PowerMockito.doNothing().when(initNetworkConfigMapper).updateInitConfig(any());
        initStatusService.setInitProgressRate(1, "210A250DSD520EWFD52VG1");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：测试初始化备份网络成功
     * 前置条件：NA
     * 检查点：初始化过程中不报错
     */
    @Test
    public void testClrInitConfigStatus() {
        ArrayList<InitConfigInfo> initConfigInfos5 = new ArrayList<>();
        InitConfigInfo initConfigInfo5 = new InitConfigInfo();
        initConfigInfo5.setInitValue("[1,10]");
        initConfigInfos5.add(initConfigInfo5);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.INIT_ERROR_CODE_PARAM))
            .thenReturn(initConfigInfos5);
        initStatusService.clrInitConfigStatus("123");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：测试初始化备份网络成功
     * 前置条件：NA
     * 检查点：返回错误码为2
     */
    @Test
    public void testQueryInitStatus() throws InstantiationException, IllegalAccessException {
        ArrayList<InitConfigInfo> initConfigInfos1 = new ArrayList<>();
        InitConfigInfo initConfigInfo1 = new InitConfigInfo();
        initConfigInfo1.setInitValue("1");
        initConfigInfos1.add(initConfigInfo1);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(Constants.INIT_ERROR_FLAG))
            .thenReturn(initConfigInfos1);
        RedissonMap<Object, Object> redissonMap = PowerMockito.mock(RedissonMap.class);
        PowerMockito.when(redisson.getMap(InitConfigConstant.INIT_RUNNING_FLAG)).thenReturn(redissonMap);
        ConfigStatus configStatus = initStatusService.queryInitStatus();
        Assert.assertEquals(configStatus.getStatus(), InitConfigConstant.ERROR_CODE_YES);
    }

    /**
     * 用例场景：测试初始化查不到配置信息
     * 前置条件：NA
     * 检查点：返回错误码为2
     */
    @Test
    public void testQueryInitStatusWithNoConfigInfo() throws InstantiationException, IllegalAccessException {
        ArrayList<InitConfigInfo> initConfigInfos1 = new ArrayList<>();
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(Constants.INIT_ERROR_FLAG))
            .thenReturn(initConfigInfos1);
        RedissonMap<Object, Object> redissonMap = PowerMockito.mock(RedissonMap.class);
        PowerMockito.when(redisson.getMap(InitConfigConstant.INIT_RUNNING_FLAG)).thenReturn(redissonMap);
        ConfigStatus configStatus = initStatusService.queryInitStatus();
        Assert.assertEquals(configStatus.getStatus(), InitConfigConstant.ERROR_CODE_YES);
    }

    /**
     * 用例场景：测试查询初始化时状态为失败
     * 前置条件：NA
     * 检查点：返回错误码为0
     */
    @Test
    public void testQueryInitStatusWithFailCode() throws InstantiationException, IllegalAccessException {
        ArrayList<InitConfigInfo> initConfigInfos1 = new ArrayList<>();
        InitConfigInfo initConfigInfo1 = new InitConfigInfo();
        initConfigInfo1.setInitValue(String.valueOf(InitConfigConstant.ERROR_CODE_NO));
        initConfigInfos1.add(initConfigInfo1);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(Constants.INIT_ERROR_FLAG))
            .thenReturn(initConfigInfos1);
        RedissonMap<Object, Object> redissonMap = PowerMockito.mock(RedissonMap.class);
        PowerMockito.when(redisson.getMap(InitConfigConstant.INIT_RUNNING_FLAG)).thenReturn(redissonMap);
        ConfigStatus configStatus = initStatusService.queryInitStatus();
        Assert.assertEquals(configStatus.getStatus(), Constants.ERROR_CODE_OK);
    }

    /**
     * 用例场景：测试初始化备份网络成功
     * 前置条件：满足条件
     * 检查点：成功
     */
    @Test
    public void testQueryInitStatusWithRunningCode() throws InstantiationException, IllegalAccessException {
        ArrayList<InitConfigInfo> initConfigInfos1 = new ArrayList<>();
        InitConfigInfo initConfigInfo1 = new InitConfigInfo();
        initConfigInfo1.setInitValue(String.valueOf(InitConfigConstant.ERROR_CODE_RUNNING));
        initConfigInfos1.add(initConfigInfo1);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(Constants.INIT_ERROR_FLAG))
            .thenReturn(initConfigInfos1);
        RedissonMap<Object, Object> redissonMap = PowerMockito.mock(RedissonMap.class);
        PowerMockito.when(redissonMap.get(any())).thenReturn("1");
        PowerMockito.when(redisson.getMap(InitConfigConstant.INIT_RUNNING_FLAG)).thenReturn(redissonMap);
        ConfigStatus configStatus = initStatusService.queryInitStatus();
        Assert.assertEquals(configStatus.getStatus(), InitConfigConstant.ERROR_CODE_RUNNING);
    }
}