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
package openbackup.system.base.sdk.dee.model;

import lombok.Data;

/**
 * 副本相关的快照信息
 *
 * @author jwx701567
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-01-20
 */
@Data
public class Snapshot {
    /**
     * 快照id
     */
    private String id;

    /**
     * 文件系统名
     */
    private String parentName;
}
