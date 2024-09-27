/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.agent.dto;

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentWwpnInfo;

import org.junit.Assert;
import org.junit.Test;

/**
 * AgentWwpnInfo测试类
 *
 * @author l30023229
 * @since 2023-02-21
 */
public class AgentWwpnInfoTest {
    /**
     * 测试对象的属性，方法（hash, equals）
     */
    @Test
    public void test() {
        EqualsVerifier.simple().forClass(AgentWwpnInfo.class).withIgnoredFields("errorCode","errorMessage").verify();
        EqualsVerifier.simple().forClass(AgentWwpnInfo.class).withIgnoredFields("errorCode","errorMessage").usingGetClass().verify();
        Assert.assertTrue(true);
    }
}
