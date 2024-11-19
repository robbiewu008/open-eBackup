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

import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import openbackup.data.protection.access.provider.sdk.resource.model.ProtectedResourceExtendInfo;

import java.util.Map;

/**
 * Protected Resource Extend Info Bo
 *
 */
@TableName(ProtectedResourceExtendInfoPo.TABLE_NAME)
public class ProtectedResourceExtendInfoPo {
    /**
     * Table name of ProtectedResourceExtendInfoPo
     */
    public static final String TABLE_NAME = "res_extend_info";

    /**
     * t_resource_set_r_resource_object表
     */
    public static final String RESOURCE_OBJECT_TABLE_NAME = "t_resource_set_r_resource_object ";

    @TableId
    private String uuid;
    private String resourceId;
    private String key;
    private String value;

    /**
     * create ProtectedResourceExtendInfoPo from entry
     *
     * @param entry entry
     * @return ProtectedResourceExtendInfoPo
     */
    public static ProtectedResourceExtendInfoPo fromEntry(Map.Entry<String, String> entry) {
        ProtectedResourceExtendInfoPo extendInfoPo = new ProtectedResourceExtendInfoPo();
        extendInfoPo.setKey(entry.getKey());
        extendInfoPo.setValue(entry.getValue());
        return extendInfoPo;
    }

    /**
     * convert to ProtectedResourceExtendInfo
     *
     * @return ProtectedResourceExtendInfo
     */
    public ProtectedResourceExtendInfo toProtectedResourceExtendInfo() {
        return ProtectedResourceExtendInfo.builder()
                .uuid(this.getUuid())
                .key(this.getKey())
                .value(this.getValue())
                .resourceId(this.getResourceId())
                .build();
    }

    public String getUuid() {
        return uuid;
    }

    public void setUuid(String uuid) {
        this.uuid = uuid;
    }

    public String getResourceId() {
        return resourceId;
    }

    public void setResourceId(String resourceId) {
        this.resourceId = resourceId;
    }

    public String getKey() {
        return key;
    }

    public void setKey(String key) {
        this.key = key;
    }

    public String getValue() {
        return value;
    }

    public void setValue(String value) {
        this.value = value;
    }
}
