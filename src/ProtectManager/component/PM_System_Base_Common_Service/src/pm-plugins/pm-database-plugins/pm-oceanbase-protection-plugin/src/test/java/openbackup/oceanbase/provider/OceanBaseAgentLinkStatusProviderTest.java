/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.oceanbase.provider;

import static org.assertj.core.api.Assertions.assertThat;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.List;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-08-14
 */
@RunWith(PowerMockRunner.class)
public class OceanBaseAgentLinkStatusProviderTest {
    private OceanBaseAgentLinkStatusProvider oceanBaseAgentLinkStatusProvider;

    @Before
    public void init() {
        oceanBaseAgentLinkStatusProvider = new OceanBaseAgentLinkStatusProvider();
    }

    /**
     * 用例场景：过滤
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource nodeResource1 = new ProtectedResource();
        nodeResource1.setSubType(ResourceSubTypeEnum.OCEAN_BASE_TENANT.getType());
        boolean applicable = oceanBaseAgentLinkStatusProvider.applicable(nodeResource1);
        Assert.assertTrue(applicable);
    }

    /**
     * 用例场景：过滤
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void get_link_status_order_success() {
        List<LinkStatusEnum> list = oceanBaseAgentLinkStatusProvider.getLinkStatusOrderList();
        List<LinkStatusEnum> expected = Lists.newArrayList(LinkStatusEnum.ONLINE, LinkStatusEnum.OFFLINE);
        assertThat(list).usingRecursiveComparison().isEqualTo(expected);
    }
}
