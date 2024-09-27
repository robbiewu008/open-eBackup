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
package openbackup.gaussdbt.protection.access.provider.util;

import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.gaussdbt.protection.access.provider.util.GaussDBTValidator;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;

/**
 * GaussDBT校验工具测试类
 *
 * @author mwx776342
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-19
 */
public class GaussDBTValidatorTest {
    /**
     * 用例场景：校验GaussDBTName
     * 前置条件：GaussDBTName正确
     * 检查点：成功
     */
    @Test
    public void check_guassdbt_name_success() {
        GaussDBTValidator.verifyName("asd");
    }

    /**
     * 用例场景：校验GaussDBTName
     * 前置条件：GaussDBTName错误
     * 检查点：失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_gaussdbt_name_is_error_when_verify() {
        Assert.assertThrows(LegoCheckedException.class, () -> GaussDBTValidator.verifyName("./asd"));
    }

    /**
     * 用例场景：校验GaussDBT注册数量
     * 前置条件：GaussDBT数量正常
     * 检查点：成功
     */
    @Test
    public void check_guassdbt_count_success() {
        GaussDBTValidator.verifyCount(GaussDBTConstant.GAUSSDBT_CLUSTER_MAX_COUNT - 1);
    }

    /**
     * 用例场景：校验GaussDBT注册数量
     * 前置条件：GaussDBT注册数量超过最大数量
     * 检查点：失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_gaussdbt_count_exceed_max_when_verify() {
        Assert.assertThrows(LegoCheckedException.class,
            () -> GaussDBTValidator.verifyCount(GaussDBTConstant.GAUSSDBT_CLUSTER_MAX_COUNT + 1));
    }
}