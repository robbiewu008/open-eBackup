/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.dto.instance;

import openbackup.tdsql.resources.access.dto.instance.Group;

import nl.jqno.equalsverifier.EqualsVerifier;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-06-25
 */
public class GroupTest {
    /**
     * 用例场景：测试Group类
     * 前置条件：equals()和hashCode()方法正确
     * 检查点：校验通过
     */
    @Test
    public void test_group() {
        EqualsVerifier.simple().forClass(Group.class).verify();
        EqualsVerifier.simple().forClass(Group.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}
