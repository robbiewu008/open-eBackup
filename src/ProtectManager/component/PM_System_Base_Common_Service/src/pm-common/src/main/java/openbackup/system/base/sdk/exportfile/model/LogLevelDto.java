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
package openbackup.system.base.sdk.exportfile.model;

import lombok.Data;

/**
 * LogLevelInfoDto
 *
 * @author w00607005
 * @since 2023-07-24
 */
@Data
public class LogLevelDto {
    /**
     * esn
     */
    private String esn;

    /**
     * 节点名称
     */
    private String nodeName;

    /**
     * 节点ip
     */
    private String nodeIp;

    /**
     * 节点角色
     */
    private int role;

    /**
     * 节点状态
     */
    private int status;

    /**
     * 日志等级
     */
    private String logLevel;
}
