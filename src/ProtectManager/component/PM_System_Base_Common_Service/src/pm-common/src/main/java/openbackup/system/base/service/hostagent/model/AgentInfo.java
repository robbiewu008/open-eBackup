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
package openbackup.system.base.service.hostagent.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;
import lombok.ToString;

import org.hibernate.validator.constraints.Length;

import java.util.Map;

import javax.validation.constraints.Max;

/**
 * Agent信息类
 *
 * @author q00654632
 * @since 2023-10-08
 */
@Getter
@Setter
@ToString(exclude = {"password"})
public class AgentInfo {
    /**
     * 资源UUID
     */
    @Length(max = 64)
    private String uuid;

    /**
     * 资源名称
     */
    @Length(max = 512)
    private String name;

    /**
     * 资源类型（主类）
     */
    @Length(max = 64)
    private String type;

    /**
     * 资源子类
     */
    @Length(max = 64)
    private String subType;

    /**
     * 资源路径
     */
    @Length(max = 1024)
    private String path;

    /**
     * 创建时间
     */
    private String createdTime;

    /**
     * 父资源名称
     */
    @Length(max = 256)
    private String parentName;

    /**
     * 父资源uuid
     */
    @Length(max = 64)
    private String parentUuid;

    /**
     * 受保护环境uuid
     */
    @Length(max = 64)
    private String rootUuid;

    /**
     * 资源的来源: restore、livemount、autoscan、register
     */
    @Length(max = 16)
    private String sourceType;

    /**
     * 资源的版本信息
     */
    @Length(max = 64)
    private String version;

    /**
     * 受保护状态
     */
    @Max(Integer.MAX_VALUE)
    private Integer protectionStatus;

    /**
     * 资源的扩展属性
     */
    private Map<String, String> extendInfo;

    /**
     * 资源所属的用户
     */
    @Length(max = 255)
    private String userId;

    /**
     * 资源授权的用户名称
     */
    @Length(max = 255)
    private String authorizedUser;

    /**
     * agent对应的ip
     */
    @Length(max = 255)
    private String endpoint;

    /**
     * agent对应端口
     */
    @Max(Integer.MAX_VALUE)
    private Integer port;

    private String linkStatus;

    private String username;

    private String password;

    private String location;

    private String osType;

    private String osName;

    @JsonProperty("cluster")
    private Boolean isCluster;

    /**
     * 定时扫描环境的时间间隔，单位为秒, 默认值为1h的秒数
     */
    private Integer scanInterval = 3600;

    private AgentExtend protectedAgentExtend;
}
