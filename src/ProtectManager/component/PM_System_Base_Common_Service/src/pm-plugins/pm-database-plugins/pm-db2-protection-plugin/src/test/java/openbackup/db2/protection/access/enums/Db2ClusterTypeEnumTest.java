/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.db2.protection.access.enums;

import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-02-02
 */
public class Db2ClusterTypeEnumTest {
    /**
     * 用例场景：db2集群枚举值未定义
     * 前置条件：枚举值未定义
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_IllegalArgumentException_if_enum_is_not_defined_when_get_by_type() {
        IllegalArgumentException illegalArgumentException = Assert.assertThrows(IllegalArgumentException.class,
            () -> Db2ClusterTypeEnum.getByType("test"));
        Assert.assertEquals(null, illegalArgumentException.getMessage());
    }
}