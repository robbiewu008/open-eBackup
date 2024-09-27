/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.opengauss.resources.access.sla;

import openbackup.database.base.plugin.service.impl.SlaValidService;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;

import openbackup.opengauss.resources.access.sla.OpenGaussSlaValidateProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

/**
 * OpenGaussSlaValidateProvider测试类
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-30
 */
public class OpenGaussSlaValidateProviderTest {
    /**
     * 用例名称：修改sla时，该sla多个策略需要满足最小集合关系
     * 前置条件：无
     * check点：修改sla时，该sla多个策略需要满足最小集合关系
     */
    @Test
    public void validate_success() {
        SlaValidService slaValidService = PowerMockito.mock(SlaValidService.class);
        SlaBase slaBase = new SlaBase();
        OpenGaussSlaValidateProvider openGaussSlaValidateProvider = new OpenGaussSlaValidateProvider(slaValidService);
        openGaussSlaValidateProvider.validateSLA(slaBase);
        Mockito.verify(slaValidService, Mockito.times(1))
            .modifySlaCheckResourcePublicPolicy(slaBase, ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType(),
                PolicyAction.DIFFERENCE_INCREMENT.getAction());
    }

    /**
     * 用例场景：openGauss sla检查类过滤
     * 前置条件：无
     * 检查点：过滤失败
     */
    @Test
    public void applicable_success() {
        SlaValidService slaValidService = PowerMockito.mock(SlaValidService.class);
        OpenGaussSlaValidateProvider openGaussSlaValidateProvider = new OpenGaussSlaValidateProvider(slaValidService);
        Assert.assertFalse(openGaussSlaValidateProvider.applicable(ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType()));
    }
}
