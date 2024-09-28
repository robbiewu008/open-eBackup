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
package openbackup.data.protection.access.provider.sdk.base.v2;

import java.util.List;
import java.util.Map;

/**
 * 存储库基本信息
 *
 */
public class BaseStorageRepository {
    /**
     * 存储库id
     */
    private String id;

    /**
     * 0-META_REPOSITORY，1-DATA_REPOSITORY，2-CACHE_REPOSITORY
     */
    private Integer type;

    /**
     * 存储协议，具体定义参考{@see RepositoryProtocolEnum}
     */
    private Integer protocol;

    /**
     * 当前仓库的角色，0-master，1-slave, 默认配置 0
     */
    private Integer role = 0;

    /**
     * 存储仓路径
     */
    private List<RemotePath> remotePath;

    private Map<String, Object> extendInfo;

    /**
     * 默认构造函数
     */
    public BaseStorageRepository() {
    }

    /**
     * 构造方法
     *
     * @param id 存储库ID
     * @param type 存储库的类型
     * @param protocol 存储库协议
     * @param extendInfo 存储仓扩展信息
     */
    public BaseStorageRepository(String id, Integer type, Integer protocol, Map<String, Object> extendInfo) {
        this.id = id;
        this.type = type;
        this.protocol = protocol;
        this.extendInfo = extendInfo;
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public Integer getType() {
        return type;
    }

    public void setType(Integer type) {
        this.type = type;
    }

    public Integer getProtocol() {
        return protocol;
    }

    public void setProtocol(Integer protocol) {
        this.protocol = protocol;
    }

    public Map<String, Object> getExtendInfo() {
        return extendInfo;
    }

    public void setExtendInfo(Map<String, Object> extendInfo) {
        this.extendInfo = extendInfo;
    }

    public Integer getRole() {
        return role;
    }

    public void setRole(Integer role) {
        this.role = role;
    }

    public List<RemotePath> getRemotePath() {
        return remotePath;
    }

    public void setRemotePath(List<RemotePath> remotePath) {
        this.remotePath = remotePath;
    }
}
