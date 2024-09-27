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
package openbackup.system.base.common.constants;

/**
 * 跨控制器转发http请求相关的常量
 *
 * @author z00850125
 * @since 2024-04-19
 */
public class RequestForwardRetryConstant {
    /**
     * 仅内部用于标识某次转发其他节点进行重试，key本身命名不会持久化，即使后续改名也不涉及演进的兼容性问题
     */
    public static final String HTTP_HEADER_INTERNAL_RETRY = "internal-retry";
}
