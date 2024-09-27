/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

package openbackup.database.base.plugin.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.AppConf;
import openbackup.database.base.plugin.util.GeneralDbUtil;
import openbackup.database.base.plugin.util.TestConfHelper;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 描述
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-03-16
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {GeneralDbUtil.class})
public class GeneralDbResourceConnectionCheckerTest {
    /**
     * 用例场景：解析资源的host信息
     * 前置条件：无
     * 检查点：查看解析出来的host信息是否正确
     */
    @Test
    public void collect_connectable_resources_can_success() {
        GeneralDbResourceConnectionChecker checker = new GeneralDbResourceConnectionChecker(null, null);

        ProtectedEnvironment singleDb = TestConfHelper.mockInstance(false);

        Map<ProtectedResource, List<ProtectedEnvironment>> resourceListMap = checker.collectConnectableResources(
            singleDb);
        Assert.assertEquals(resourceListMap.keySet().size(), 2);
    }

    /**
     * 用例场景：对检查连通性结果进行处理
     * 前置条件：无
     * 检查点：处理结果正确
     */
    @Test
    public void test_collectActionResults() {
        GeneralDbResourceConnectionChecker checker = new GeneralDbResourceConnectionChecker(null, null);
        List<CheckReport<Object>> checkReportList = new ArrayList<>();
        CheckReport checkReport = PowerMockito.mock(CheckReport.class);
        checkReportList.add(checkReport);

        ProtectedResource protectedResource = PowerMockito.mock(ProtectedResource.class);
        PowerMockito.when(protectedResource.getExtendInfoByKey(any())).thenReturn("");

        PowerMockito.when(checkReport.getResource()).thenReturn(protectedResource);

        AppConf appConf = PowerMockito.mock(AppConf.class);
        PowerMockito.mockStatic(GeneralDbUtil.class);
        PowerMockito.when(GeneralDbUtil.getAppConf("")).thenReturn(Optional.ofNullable(appConf));

        List<CheckResult<Object>> checkResultList = new ArrayList<>();
        CheckResult<Object> checkResult = new CheckResult<>();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(ActionResult.SUCCESS_CODE);
        checkResult.setResults(actionResult);

        checkResultList.add(checkResult);

        PowerMockito.when(checkReport.getResults()).thenReturn(checkResultList);

        Map<String, Object> context = new HashMap<>();

        List<ActionResult> actionResults = checker.collectActionResults(checkReportList, context);
        Assert.assertEquals(1, actionResults.size());
    }
}
