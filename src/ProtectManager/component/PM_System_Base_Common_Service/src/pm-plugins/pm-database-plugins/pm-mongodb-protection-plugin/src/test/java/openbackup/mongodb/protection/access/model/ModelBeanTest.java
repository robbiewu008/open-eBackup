/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
package openbackup.mongodb.protection.access.model;


import openbackup.mongodb.protection.access.enums.MongoDBClusterRoleEnum;
import openbackup.mongodb.protection.access.enums.MongoDBClusterTypeEnum;
import openbackup.mongodb.protection.access.enums.MongoDBNodeTypeEnum;

import lombok.Data;
import nl.jqno.equalsverifier.EqualsVerifier;

import org.junit.Assert;
import org.junit.Test;

/**
 * MongoDBbean对象创建类
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
 */

@Data
public class ModelBeanTest {
    /**
     * 用例场景：测试MongoDBClusterRoleEnum类的equals和hashcode方法
     * 前置条件：无
     * 检查点：是否报错
     */
    @Test
    public void create_mongodb_cluster_role_enum() {
        EqualsVerifier.simple().forClass(MongoDBClusterRoleEnum.class).verify();
        EqualsVerifier.simple().forClass(MongoDBClusterRoleEnum.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
    /**
     * 用例场景：测试MongoDBClusterTypeEnum类的equals和hashcode方法
     * 前置条件：无
     * 检查点：是否报错
     */
    @Test
    public void create_mongodb_cluster_type_enum() {
        EqualsVerifier.simple().forClass(MongoDBClusterTypeEnum.class).verify();
        EqualsVerifier.simple().forClass(MongoDBClusterTypeEnum.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
    /**
     * 用例场景：测试MongoDBNodeTypeEnum类的equals和hashcode方法
     * 前置条件：无
     * 检查点：是否报错
     */
    @Test
    public void create_mongodb_node_type_enum() {
        EqualsVerifier.simple().forClass(MongoDBNodeTypeEnum.class).verify();
        EqualsVerifier.simple().forClass(MongoDBNodeTypeEnum.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}
