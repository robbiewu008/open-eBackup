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
package openbackup.oracle.bo;

import openbackup.data.protection.access.provider.sdk.base.Authentication;

import lombok.Getter;
import lombok.Setter;

import java.util.Map;

/**
 * 功能描述
 *
 */
@Getter
@Setter
public class OracleStorageSnapshotNode {
    /**
     * 资源ID
     */
    private String id;

    /**
     * 资源名称
     */
    private String name;

    /**
     * 资源类型
     */
    private String type;

    /**
     * 资源子类型
     */
    private String subType;

    /**
     * 访问地址，ip或者域名
     */
    private String endpoint;

    /**
     * 端口
     */
    private Integer port;

    /**
     * 环境的认证信息
     */
    private Authentication auth;

    /**
     * 资源的扩展信息
     */
    private Map<String, String> extendInfo;
}
