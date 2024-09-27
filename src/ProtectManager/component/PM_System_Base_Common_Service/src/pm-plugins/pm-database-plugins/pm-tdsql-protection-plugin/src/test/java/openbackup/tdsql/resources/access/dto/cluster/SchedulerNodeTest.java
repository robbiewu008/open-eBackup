/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.dto.cluster;

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.tdsql.resources.access.dto.cluster.SchedulerNode;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-06-25
 */
public class SchedulerNodeTest {
    /**
     * 用例场景：测试SchedulerNode类
     * 前置条件：equals()和hashCode()方法正确
     * 检查点：校验通过
     */
    @Test
    public void test_scheduler_node() {
        EqualsVerifier.simple().forClass(SchedulerNode.class).verify();
        EqualsVerifier.simple().forClass(SchedulerNode.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}
