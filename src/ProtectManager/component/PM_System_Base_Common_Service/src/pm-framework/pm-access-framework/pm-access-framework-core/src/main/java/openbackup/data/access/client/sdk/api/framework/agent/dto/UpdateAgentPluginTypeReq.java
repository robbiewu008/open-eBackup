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
package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Getter;
import lombok.Setter;

/**
 * 更新代理资源类型请求
 *
 * @author c30035089
 * @since 2023-08-11
 */
@Getter
@Setter
public class UpdateAgentPluginTypeReq {
    /**
     * 下载链接
     */
    private String downloadLink;

    /**
     * agent id
     */
    private String agentId;

    /**
     * agent name
     */
    private String agentName;

    /**
     * 任务id
     */
    private String jobId;

    /**
     * 密钥,已加密
     */
    private String certSecretKey;

    /**
     * 安装包大小(kb)
     */
    private long newPackageSize;
}
