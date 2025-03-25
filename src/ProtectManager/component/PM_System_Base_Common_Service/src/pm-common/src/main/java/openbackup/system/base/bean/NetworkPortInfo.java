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
package openbackup.system.base.bean;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 功能描述：存放E1000的网口信息
 *
 */
@Getter
@Setter
public class NetworkPortInfo {
    /**
     * 网口名称
     */
    private String ifaceName;

    /**
     * 对应网口的ip列表
     */
    private List<String> ipList;
}
