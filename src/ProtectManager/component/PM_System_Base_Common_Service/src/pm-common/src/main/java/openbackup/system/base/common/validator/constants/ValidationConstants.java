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
package openbackup.system.base.common.validator.constants;

/**
 * 验证错误码类
 *
 * @author w00448845
 * @version [CDM Integrated machine]
 * @since 2019-11-05
 */
public class ValidationConstants {
    /**
     * MTU最小值
     */
    public static final int MIN_MTU = 1280;

    /**
     * MTU最大值
     */
    public static final int MAX_MTU = 9600;

    /**
     * MTU校验错误返回message信息
     */
    public static final String WRONG_MTU_MESSAGE = "Please input mtu in range [" + MIN_MTU + "," + MAX_MTU + "]";
}
