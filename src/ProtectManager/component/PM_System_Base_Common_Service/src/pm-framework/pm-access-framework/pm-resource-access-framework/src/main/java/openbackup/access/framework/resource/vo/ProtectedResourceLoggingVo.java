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
package openbackup.access.framework.resource.vo;

import lombok.Getter;
import lombok.Setter;

/**
 * ProtectedResourceLoggingVo resource 事件vo对象，主要用于组装后返回给logging框架记录detail
 *
 */
@Getter
@Setter
public class ProtectedResourceLoggingVo {
    /**
     * 默认信息内容 --
     */
    public static final String DEFAULT_VAL = "--";

    private String storageName = DEFAULT_VAL;

    private String storageUUID = DEFAULT_VAL;

    private String storageType = DEFAULT_VAL;

    private String tenantName = DEFAULT_VAL;

    private String tenantId = DEFAULT_VAL;

    private String resourceName = DEFAULT_VAL;

    private String resourceId = DEFAULT_VAL;
}
