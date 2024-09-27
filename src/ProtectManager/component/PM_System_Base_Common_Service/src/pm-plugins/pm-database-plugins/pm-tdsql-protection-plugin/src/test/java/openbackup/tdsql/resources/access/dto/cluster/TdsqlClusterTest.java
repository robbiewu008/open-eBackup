/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.dto.cluster;

import openbackup.tdsql.resources.access.dto.cluster.TdsqlCluster;

import nl.jqno.equalsverifier.EqualsVerifier;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-06-25
 */
public class TdsqlClusterTest {
    /**
     * 用例场景：测试TdsqlCluster类
     * 前置条件：equals()和hashCode()方法正确
     * 检查点：校验通过
     */
    @Test
    public void test_tdsql_cluster() {
        EqualsVerifier.simple().forClass(TdsqlCluster.class).verify();
        EqualsVerifier.simple().forClass(TdsqlCluster.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}
