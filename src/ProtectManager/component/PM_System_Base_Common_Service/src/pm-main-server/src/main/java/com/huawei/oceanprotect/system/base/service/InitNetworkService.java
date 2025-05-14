/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service;

import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.api.StorageArraySessionResponse;
import com.huawei.oceanprotect.system.base.vo.InitNetWorkParam;

import openbackup.system.base.util.Applicable;

/**
 * 初始化时，定义不同部署类型操作不同的接口
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-03
 */
public interface InitNetworkService extends Applicable<String> {
    /**
     * 获取初始化的网络参数
     *
     * @param initNetworkBody 请求参数
     * @param deviceId deviceId
     * @param username username
     * @return InitNetWorkParam
     */
    InitNetWorkParam getInitNetWorkParam(InitNetworkBody initNetworkBody, String deviceId, String username);

    /**
     * 统一参数校验
     *
     * @param deviceId deviceId
     * @param username username
     * @param initNetworkBody 初始化参数
     */
    void unifiedCheck(String deviceId, String username, InitNetworkBody initNetworkBody);

    /**
     * 统一参数校验
     *
     * @param deviceId deviceId
     * @param username username
     * @param initNetworkBody 初始化参数
     * @param isLld 是否是解析lld时检查参数
     */
    default void unifiedCheck(String deviceId, String username, InitNetworkBody initNetworkBody, boolean isLld) {}

    /**
     * 获取设备ip用于openstorageapi访问设备
     *
     * @return deviceIp
     */
    String getDeviceIp();

    /**
     * 获取设备类型
     *
     * @return deviceIp
     */
    String getDeviceType();

    /**
     * 新建机机账号时，获取用户角色id
     *
     * @return 用户角色id
     */
    Integer getStorageUserRoleId();

    /**
     * 获取超级管理员角色id
     *
     * @return 超级管理员角色id
     */
    Integer getSuperAdminRoleId();

    /**
     * 获取设备session时检查返回结果
     *
     * @param arraySessionResponse arraySessionResponse
     * @param username username
     * @param isAllowUnInitUser isAllowUnInitUser
     */
    void checkLoginResponse(StorageArraySessionResponse arraySessionResponse,
        String username, boolean isAllowUnInitUser);

    /**
     * 创建存储用户
     *
     * @param authUsername 已经认证过的用户名
     * @param addUsername 待创建的用户名
     * @param addUserRoleId 角色id
     * @return 获取创建之后密码
     */
    String createStorageUser(String authUsername, String addUsername, Integer addUserRoleId);

    /**
     * 初始化存储用户
     *
     * @param addUsername 待创建的用户名
     */
    void initStorageUser(String addUsername);

    /**
     * 修改密码
     *
     * @param addUsername 待修改的用户名
     * @param password 当前的password
     * @return 修改之后的密码
     */
    String changePass(String addUsername, String password);

    /**
     * 添加逻辑端口
     *
     * @param initNetworkBody 请求参数
     */
    void addLogicPort(InitNetworkBody initNetworkBody);

    /**
     * 添加端口路由
     *
     * @param logicPortDto 逻辑端口
     */
    void addPortRoute(LogicPortDto logicPortDto);

    /**
     * 初始化操作日志存储策略
     */
    void initOperationLogStrategy();
}
