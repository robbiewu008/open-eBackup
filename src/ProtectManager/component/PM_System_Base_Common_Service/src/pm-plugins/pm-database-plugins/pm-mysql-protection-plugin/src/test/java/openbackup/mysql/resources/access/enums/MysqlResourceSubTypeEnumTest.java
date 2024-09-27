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
package openbackup.mysql.resources.access.enums;

import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;

import org.junit.Test;

/**
 * MysqlResourceSubTypeEnum测试类
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/5/31
 */
public class MysqlResourceSubTypeEnumTest {
    /**
     * 用例场景：检验资源类型是否属于MySQL
     * 前置条件：1. 资源类型属于MySQL
     * 检 查 点：1. 返回值为true
     */
    @Test
    public void check_is_belong_to_mysql_resource_success_if_resource_is_mysql_database() {
        assert MysqlResourceSubTypeEnum.isBelongToMysql(MysqlResourceSubTypeEnum.MYSQL_CLUSTER.getType());
        assert MysqlResourceSubTypeEnum.isBelongToMysql(MysqlResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType());
        assert MysqlResourceSubTypeEnum.isBelongToMysql(MysqlResourceSubTypeEnum.MYSQL_DATABASE.getType());
    }

    /**
     * 用例场景：检验资源类型是否属于MySQL
     * 前置条件：1. 资源类型属于MySQL
     * 检 查 点：1. 返回值为true
     */
    @Test
    public void check_is_belong_to_mysql_resource_success_if_resource_is_not_mysql_database() {
        assert !MysqlResourceSubTypeEnum.isBelongToMysql("gauss");
    }
}
