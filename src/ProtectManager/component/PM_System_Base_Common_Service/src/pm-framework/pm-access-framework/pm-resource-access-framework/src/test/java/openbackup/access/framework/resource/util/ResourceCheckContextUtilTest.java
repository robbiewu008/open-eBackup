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
package openbackup.access.framework.resource.util;

import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.util.ResourceCheckContextUtil;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * ResourceCheckContextUtil
 */
public class ResourceCheckContextUtilTest {
    /**
     * 用例名称：验证资源检查上下文对象正确性。<br/>
     * 前置条件：资源检查上下文初始化成功。<br/>
     * check点：<br/>
     * 1、检查成功场景不报错；<br/>
     * 2、检查失败场景抛出联合错误。<br/>
     */
    @Test
    public void check() {
        ResourceCheckContext context = new ResourceCheckContext();
        List<ActionResult> results = new ArrayList<>();
        results.add(new ActionResult());
        context.setActionResults(results);
        ResourceCheckContextUtil.check(context, "check failed");
        ActionResult result = new ActionResult();
        result.setCode(1);
        result.setDetailParams(Arrays.asList("a1", "b1"));
        result.setMessage("errorMessage1");
        results.add(result);
        ActionResult result2 = new ActionResult();
        result2.setCode(2);
        results.add(result2);
        LegoCheckedException exception =
                Assert.assertThrows(
                        LegoCheckedException.class, () -> ResourceCheckContextUtil.check(context, "check failed"));
        Assert.assertEquals(ResourceCheckContextUtil.UNION_ERROR, exception.getErrorCode());
        String[] parameters = exception.getParameters();
        Assert.assertEquals(2, parameters.length);
        Assert.assertEquals("i18n:[\"lego.err.1\",\"a1\",\"b1\"]", parameters[0]);
        Assert.assertEquals("i18n:[\"lego.err.2\"]", parameters[1]);
        Assert.assertEquals("check failed", exception.getMessage());
    }
}
