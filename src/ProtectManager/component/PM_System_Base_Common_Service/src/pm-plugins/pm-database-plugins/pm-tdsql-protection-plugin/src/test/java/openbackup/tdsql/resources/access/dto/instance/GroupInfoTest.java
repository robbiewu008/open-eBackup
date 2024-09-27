/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.dto.instance;

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.tdsql.resources.access.dto.instance.GroupInfo;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述 测试GroupInfo类
 *
 * @author z00445440
 * @since 2023-11-16
 */
public class GroupInfoTest {
    @Test
    public void test_group_info() {
        EqualsVerifier.simple().forClass(GroupInfo.class).verify();
        EqualsVerifier.simple().forClass(GroupInfo.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}
