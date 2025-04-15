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

import com.huawei.emeistor.kms.kmc.util.KmcHelper;

import com.fasterxml.jackson.annotation.JsonIgnore;

import lombok.Data;
import openbackup.data.protection.access.provider.sdk.base.Authentication;

import java.util.HashMap;
import java.util.Map;

/**
 * RestoreCommonResource
 *
 * @description: 恢复任务中通用的资源信息，是资源类和环境类中公共的信息
 **/
@Data
abstract class TaskCommonResource {
    /**
     * 资源ID
     */
    private String uuid;

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
     * 资源路径
     */
    private String path;

    /**
     * 受保护环境uuid
     */
    private String rootUuid;

    /**
     * 资源的版本信息
     */
    @JsonIgnore
    private String version;

    /**
     * 资源的扩展信息
     */
    private Map<String, String> extendInfo;

    /**
     * 环境的认证信息
     */
    private Authentication auth;

    /**
     * 加密auth里面的authPwd密码
     */
    public void encryptPassword() {
        if (auth != null) {
            auth.setAuthPwd(KmcHelper.getInstance().encrypt(auth.getAuthPwd()));
        }
    }

    /**
     * 解密auth里面的authPwd密码
     */
    public void decryptPassword() {
        if (auth != null) {
            auth.setAuthPwd(KmcHelper.getInstance().decrypt(auth.getAuthPwd()));
        }
    }

    /**
     * 加密资源的扩展信息
     */
    public void encryptExtendInfo() {
        if (extendInfo != null) {
            Map<String, String> encryptMap = new HashMap<>();
            for (String key : extendInfo.keySet()) {
                String encryptValue = KmcHelper.getInstance().encrypt(extendInfo.get(key));
                encryptMap.put(key, encryptValue);
            }
            setExtendInfo(encryptMap);
        }
    }

    /**
     * 解密资源的扩展信息
     */
    public void decryptExtendInfo() {
        if (extendInfo != null) {
            Map<String, String> decryptMap = new HashMap<>();
            for (String key : extendInfo.keySet()) {
                String decryptValue = KmcHelper.getInstance().decrypt(extendInfo.get(key));
                decryptMap.put(key, decryptValue);
            }
            setExtendInfo(decryptMap);
        }
    }

    /**
     * 加密auth里面的扩展信息
     */
    public void encryptAuthExtendInfo() {
        if (auth != null && auth.getExtendInfo() != null) {
            Map<String, String> encryptMap = new HashMap<>();
            for (String key : auth.getExtendInfo().keySet()) {
                String encryptValue = KmcHelper.getInstance().encrypt(auth.getExtendInfo().get(key));
                encryptMap.put(key, encryptValue);
            }
            auth.setExtendInfo(encryptMap);
        }
    }

    /**
     * 解密auth里面的扩展信息
     */
    public void decryptAuthExtendInfo() {
        if (auth != null && auth.getExtendInfo() != null) {
            Map<String, String> decryptMap = new HashMap<>();
            for (String key : auth.getExtendInfo().keySet()) {
                String decryptValue = KmcHelper.getInstance().decrypt(auth.getExtendInfo().get(key));
                decryptMap.put(key, decryptValue);
            }
            auth.setExtendInfo(decryptMap);
        }
    }
}
