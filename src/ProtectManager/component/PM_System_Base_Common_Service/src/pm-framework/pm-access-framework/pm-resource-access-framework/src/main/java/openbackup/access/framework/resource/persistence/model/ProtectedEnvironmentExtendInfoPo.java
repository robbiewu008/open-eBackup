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
package openbackup.access.framework.resource.persistence.model;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;

import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import org.springframework.beans.BeanUtils;

/**
 * Protected Environment Extend Info Po
 *
 */
@TableName("ENVIRONMENTS")
public class ProtectedEnvironmentExtendInfoPo {
    @TableId
    private String uuid;

    private String endpoint;

    private Integer port;

    private String linkStatus;

    @TableField("user_name")
    private String username;

    private String password;

    private String location;

    private String osType;

    private String osName;

    private Boolean isCluster;

    // 定时扫描环境的时间间隔，单位为秒
    private Integer scanInterval;

    /**
     * create ProtectedEnvironmentExtendInfoPo from ProtectedEnvironment
     *
     * @param environment protected environment
     * @return ProtectedEnvironmentExtendInfoPo
     */
    public static ProtectedEnvironmentExtendInfoPo fromProtectedEnvironment(ProtectedEnvironment environment) {
        ProtectedEnvironmentExtendInfoPo protectedEnvironmentExtendInfoPo = new ProtectedEnvironmentExtendInfoPo();
        BeanUtils.copyProperties(environment, protectedEnvironmentExtendInfoPo);
        protectedEnvironmentExtendInfoPo.setCluster(environment.isCluster());
        return protectedEnvironmentExtendInfoPo;
    }

    public String getUuid() {
        return uuid;
    }

    public void setUuid(String uuid) {
        this.uuid = uuid;
    }

    public String getEndpoint() {
        return endpoint;
    }

    public void setEndpoint(String endpoint) {
        this.endpoint = endpoint;
    }

    public Integer getPort() {
        return port;
    }

    public void setPort(Integer port) {
        this.port = port;
    }

    public String getLinkStatus() {
        return linkStatus;
    }

    public void setLinkStatus(String linkStatus) {
        this.linkStatus = linkStatus;
    }

    public String getUsername() {
        return username;
    }

    public void setUsername(String username) {
        this.username = username;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public String getLocation() {
        return location;
    }

    public void setLocation(String location) {
        this.location = location;
    }

    public String getOsType() {
        return osType;
    }

    public void setOsType(String osType) {
        this.osType = osType;
    }

    public String getOsName() {
        return osName;
    }

    public void setOsName(String osName) {
        this.osName = osName;
    }

    public Integer getScanInterval() {
        return scanInterval;
    }

    public void setScanInterval(Integer scanInterval) {
        this.scanInterval = scanInterval;
    }

    public Boolean isCluster() {
        return isCluster;
    }

    public void setCluster(Boolean isCluster) {
        this.isCluster = isCluster;
    }
}
