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
package openbackup.system.base.sdk.agent.model;

import lombok.Data;

import org.apache.commons.lang3.StringUtils;

/**
 * agent更新请求体
 *
 */
@Data
public class AgentUpdateRequest {
    /**
     * host ip
     */
    private String ip;

    /**
     * host port
     */
    private String port;

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
     * 是否需要代理
     */
    private boolean canUseProxy;

    /**
     * 密钥,已加密
     */
    private String certSecretKey;

    /**
     * 任务id
     */
    private String jobId;

    /**
     * 安装包大小(kb)
     */
    private long newPackageSize;

    @Override
    public String toString() {
        final StringBuffer sb = new StringBuffer("AgentUpdateRequest{");
        sb.append("ip='").append(ip).append('\'');
        sb.append(", port='").append(port).append('\'');
        sb.append(", downloadLink'").append(downloadLink).append('\'');
        sb.append(", agentId='").append(agentId).append('\'');
        sb.append(", agentName='").append(agentName).append('\'');
        sb.append(", newPackageSize='").append(newPackageSize).append('\'');
        sb.append(", canUseProxy=").append(canUseProxy);
        sb.append(", certSecretKey exist ? =").append(StringUtils.isNotBlank(certSecretKey)).append('\'');
        sb.append('}');
        return sb.toString();
    }
}
