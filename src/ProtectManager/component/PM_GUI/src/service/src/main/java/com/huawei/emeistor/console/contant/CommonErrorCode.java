/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package com.huawei.emeistor.console.contant;

/**
 * 错误码相关类
 *
 * @since 2020-06-15
 * @author t00482481
 */
public class CommonErrorCode {
    /**
     * 用户在线错误码
     */
    public static final long USER_IS_ONLINE = 1677929484L;

    /**
     * 达到最大session错误码
     */
    public static final long LOGIN_USER_EXCEED_MAX_SESSION = 1677929492L;

    /**
     * 达到重置密码最大次数错误码
     */
    public static final long REACH_MAX_RESET_LIMIT = 1677929501L;

    /**
     * 验证码校验失败错误码
     */
    public static final long WRONG_VRF_CODE = 1677929502L;

    /**
     * 请求超时错误码
     */
    public static final long REQUEST_TIMEOUT = 1677929226L;

    /**
     * ERR_PARAM
     */
    public static final long ERR_PARAM = 1677929218L;

    /**
     * 操作失败（非提示性错误）
     */
    public static final long OPERATION_FAILED = 1677929219L;

    /**
     * 未配置SFTP服务器信息。
     */
    public static final long FILL_POLICY_FIRST = 1677934096L;

    /**
     * 管理数据备份文件格式错误或文件内容缺失。
     */
    public static final long ERROR_IMPORT_FILE = 1677934081L;

    /**
     * 用户状态异常
     */
    public static final long USER_STATUS_ABNORMAL = 1677929505L;

    /**
     * 原因：系统异常。
     * 建议：请联系技术支持工程师协助解决。
     */
    public static final long SYSTEM_ERROR = 1677929221L;

    /**
     * 超过允许同时下载数量限制
     */
    public static final long DOWNLOAD_MAX = 1677936644L;
}
