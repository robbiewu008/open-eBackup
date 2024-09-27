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
package openbackup.data.access.framework.livemount.common.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 挂载销毁扩展参数
 *
 * @author w30044259
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-11-22
 */
@Setter
@Getter
public class UnmountExtendParam {
    private String userId;

    private String exerciseId;

    private String exerciseJobId;

    /**
     * 获取扩展参数实例
     *
     * @return param
     */
    public static UnmountExtendParam getInstance() {
        return new UnmountExtendParam();
    }
}
