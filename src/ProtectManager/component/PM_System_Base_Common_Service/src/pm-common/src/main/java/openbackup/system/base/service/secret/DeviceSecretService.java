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
package openbackup.system.base.service.secret;

import openbackup.system.base.bean.DeviceUser;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraConfigMapRequest;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * secret接口实现
 *
 * @author l00853347
 * @since 2023-12-22
 * @version [OceanProtect DataBackup 1.6.0]
 */
@Service
@Slf4j
public class DeviceSecretService {
    private static final String NAMESPACE = "dpa";

    private static final String DEVICE_SECRET = "device-secret";

    private static final String SUCCESS = "success";

    @Autowired
    private InfrastructureRestApi infrastructureRestApi;

    /**
     * 查询所有设备secret
     *
     * @return 查询结果
     */
    public List<DeviceUser> queryAllSecret() {
        List<DeviceUser> deviceUserList = new ArrayList<>();
        InfraResponseWithError<JSONArray> secret;
        try {
            secret = infrastructureRestApi.getSecret(NAMESPACE, DEVICE_SECRET);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Query from infra failed:", ExceptionUtil.getErrorMessage(e));
            return deviceUserList;
        }
        JSONArray dataJsonArray = Optional.ofNullable(secret.getData()).orElse(new JSONArray());
        for (Object data : dataJsonArray) {
            JSONObject secretObject = JSONObject.fromObject(data);
            Iterator<String> deviceIterator = secretObject.keys();
            while (deviceIterator.hasNext()) {
                String key = deviceIterator.next();
                JSONArray secretList = secretObject.getJSONArray(key);
                for (JSONObject deviceUserJson : (Iterable<JSONObject>) secretList) {
                    deviceUserList.add(JSONObject.toBean(deviceUserJson, DeviceUser.class));
                }
            }
        }
        return deviceUserList;
    }

    /**
     * 查询secret
     *
     * @param deviceId 设备id
     * @return 查询结果
     */
    public List<DeviceUser> querySecret(String deviceId) {
        return queryAllSecret().stream()
                .filter(user -> user.getId().equals(deviceId))
                .collect(Collectors.toList());
    }

    /**
     * 查询指定secret
     *
     * @param deviceId 设备id
     * @param username 用户名
     * @return 查询结果
     */
    public List<DeviceUser> querySecret(String deviceId, String username) {
        return querySecret(deviceId).stream()
                .filter(user -> user.getUsername().equals(username))
                .collect(Collectors.toList());
    }

    /**
     * 新增或修改secret, 此方法不会加密密码，需要调用方自己加密
     *
     * @param deviceUser 用户账户
     * @return 创建是否成功
     */
    public boolean createSecret(DeviceUser deviceUser) {
        List<DeviceUser> deviceUsers = querySecret(deviceUser.getId());
        if (VerifyUtil.isEmpty(deviceUsers)) {
            return addSecret(deviceUser);
        }
        List<DeviceUser> users = deviceUsers.stream()
                .filter(user -> !user.getUsername().equals(deviceUser.getUsername()))
                .collect(Collectors.toList());
        users.add(deviceUser);
        return updateSecret(deviceUser.getId(), users);
    }

    /**
     * 删除secret
     *
     * @param deviceId 机器ESN号
     * @param username 用户名
     * @return 是否删除成功
     */
    public boolean deleteSecret(String deviceId, String username) {
        List<DeviceUser> deviceUsers = querySecret(deviceId);
        List<DeviceUser> users = deviceUsers.stream()
                .filter(user -> !user.getUsername().equals(username))
                .collect(Collectors.toList());
        if (VerifyUtil.isEmpty(users)) {
            return deleteSecretKey(deviceId);
        } else {
            return updateSecret(deviceId, users);
        }
    }

    /**
     * 只删除secret的key
     *
     * @param deviceId 机器ESN号
     * @return 是否删除成功
     */
    public boolean deleteSecretKey(String deviceId) {
        List<DeviceUser> deviceUsers = querySecret(deviceId);
        if (VerifyUtil.isEmpty(deviceUsers)) {
            return true;
        }
        String res = infrastructureRestApi.deleteSecret(NAMESPACE, DEVICE_SECRET, deviceId).getData();
        return SUCCESS.equals(res);
    }

    private boolean addSecret(DeviceUser deviceUser) {
        String userStr = JSONArray.fromObject(Collections.singletonList(deviceUser)).toString();
        InfraResponseWithError<String> resp = infrastructureRestApi.createSecret(
                NAMESPACE, DEVICE_SECRET, deviceUser.getId(), userStr);
        return SUCCESS.equals(resp.getData());
    }

    // 需要deviceUsers中的id相同，且都为deviceId
    private boolean updateSecret(String deviceId, List<DeviceUser> deviceUsers) {
        String userStr = JSONArray.fromObject(deviceUsers).toString();
        InfraResponseWithError<String> resp = infrastructureRestApi.updateSecret(
                new InfraConfigMapRequest(NAMESPACE, DEVICE_SECRET, deviceId, userStr));
        return SUCCESS.equals(resp.getData());
    }
}
