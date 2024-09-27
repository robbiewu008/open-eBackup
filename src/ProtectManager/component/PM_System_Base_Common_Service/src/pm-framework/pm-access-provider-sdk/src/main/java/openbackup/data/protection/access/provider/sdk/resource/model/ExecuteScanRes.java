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
package openbackup.data.protection.access.provider.sdk.resource.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 执行扫描的结果
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-06-06
 */
@Getter
@Setter
public class ExecuteScanRes {
    /**
     * 扫描出来的新的资源id
     */
    private List<String> scanNewResourceUuids;

    /**
     * 是否终止扫描流程
     * 该值为true，则后续不会使用框架的扫描逻辑
     */
    private boolean isEndExecute;

    /**
     * 构造器
     *
     * @param scanNewResourceUuids 扫描资源uuid
     * @param isEndExecute 是否完成了处理
     */
    public ExecuteScanRes(List<String> scanNewResourceUuids, boolean isEndExecute) {
        this.scanNewResourceUuids = scanNewResourceUuids;
        this.isEndExecute = isEndExecute;
    }

    /**
     * 返回默认值
     *
     * @return 默认值
     */
    public static ExecuteScanRes defaultValue() {
        return new ExecuteScanRes(null, false);
    }
}
