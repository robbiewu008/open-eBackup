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
package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 本地存储文件系统BO
 *
 */
@Getter
@Setter
public class StorageFileSystemBo {
    /**
     * ID
     */
    @JsonProperty("ID")
    private String id;

    /**
     * 名称
     */
    @JsonProperty("NAME")
    private String name;

    /**
     * 远程复制ID列表
     */
    @JsonProperty("REMOTEREPLICATIONIDS")
    private String remoteReplicationIds;

    /**
     * 双活PairID列表
     */
    @JsonProperty("HYPERMETROPAIRIDS")
    private String hyperMetroPairIds;


    /**
     * NAS协议支持多种安全模式
     * <p>
     * 1：Native安全模式
     * 2：NTFS安全模式
     * 3：UNIX安全模式
     */
    @JsonProperty("securityStyle")
    private String securityStyle;

    /**
     * 文件系统类型
     * <p>
     * 0：普通文件系统
     * 1：WORM文件系统
     */
    @JsonProperty("SUBTYPE")
    private String subType;

    /**
     * 是否是克隆文件系统
     */
    @JsonProperty("ISCLONEFS")
    private boolean isCloneFs;
}
