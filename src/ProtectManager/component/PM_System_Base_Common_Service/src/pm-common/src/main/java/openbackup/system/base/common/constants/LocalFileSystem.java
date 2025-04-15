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

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 本地文件系统
 *
 */
@Getter
@Setter
public class LocalFileSystem {
    /**
     * ID
     */
    private String id;

    /**
     * 名称
     */
    private String name;

    /**
     * 远程复制ID列表
     */
    private List<String> remoteReplicationIds;

    /**
     * 双活PairID列表
     */
    private List<String> hyperMetroPairIds;

    /**
     * NAS协议支持多种安全模式
     */
    private String securityStyle;

    /**
     * 文件系统类型
     */
    private String subType;

    /**
     * 是否是克隆文件系统
     */
    private boolean isCloneFs;

    /**
     * 是否配置了smart mobility，配置了值为 1，不配置为null
     */
    private String hasSmartMobility;
}
