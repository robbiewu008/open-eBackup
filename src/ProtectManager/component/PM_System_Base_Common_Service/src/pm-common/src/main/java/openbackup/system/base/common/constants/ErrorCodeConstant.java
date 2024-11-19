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

/**
 * 错误码常量类
 *
 */
public class ErrorCodeConstant {
    /**
     * 鉴权失败。
     */
    public static final long AUTHENTICATION_FAIL = 1677929497L;

    /**
     * 对象不存在
     */
    public static final long OBJ_NOT_EXIST = 1073947394L;

    /**
     * 通信异常
     */
    public static final long REST_CORRESPONDENCE_FAILED = 1073947647L;

    /**
     * ERROR_CODE_BASE
     */
    public static final long ERROR_CODE_BASE = 0L;

    /* 数据库操作异常 */

    /**
     * DATABASE_BASE
     */
    public static final long DATABASE_BASE = ERROR_CODE_BASE + LegoNumberConstant.HUNDRED;

    /**
     * 系统错误
     */
    public static final long SYSTEM_ERROR = DATABASE_BASE + LegoNumberConstant.TWO;

    /* 公共操作异常 */

    /**
     * SYSTEM_COMMON_BASE
     */
    public static final long SYSTEM_COMMON_BASE = ERROR_CODE_BASE + LegoNumberConstant.VALUE_200;

    /**
     * SSL连接失败或者初始化失败
     */
    public static final long SSL_INIT_OR_CONNECT_FAIL = SYSTEM_COMMON_BASE + LegoNumberConstant.SEVEN;

    /* ********************************** 用户管理模块 ******************************************** */
    /* ********************************** 任务管理模块开始 ******************************************** */
    /* 任务管理模块错误码基线 */

    /**
     * TASK_MODEL_BASE
     */
    public static final long TASK_MODEL_BASE = ERROR_CODE_BASE + LegoNumberConstant.VALUE_600;

    /* ********************************** 任务管理模块结束 ****************************************** */
    /* ********************************** 南向模块 开始 ******************************************** */
    // 南向模块错误码相对初始值，从512开始即0x00000200

    /**
     * SOUTH_MODEL_BASE
     */
    public static final long SOUTH_MODEL_BASE = ERROR_CODE_BASE + LegoNumberConstant.VALUE_512;

    /* ********************************** 南向模块 结束 ******************************************** */
    /* ********************************** Mediation模块 开始 ******************************************** */
    // 南向模块错误码相对初始值，从512开始即0x00000300

    /* ********************************** Mediation模块结束 ******************************************** */
    /* ********************************** 发现模块 开始 ******************************************** */

    /**
     * 发现模块错误码相对初始值，从1024开始即0x00000400
     */
    public static final long DISCOVERY_MODEL_BASE = ERROR_CODE_BASE + LegoNumberConstant.THROUND_TWENTY_FOUR;

    /* ********************************** 发现模块 结束 ******************************************** */
    /* ********************************** 告警模块 开始 ******************************************** */
    // 发现模块错误码相对初始值，从1536开始即0x00000600

    /**
     * FAULT_MODEL_BASE
     */
    public static final long FAULT_MODEL_BASE = ERROR_CODE_BASE + LegoNumberConstant.VALUE_1536;

    /**
     * 告警不存在
     */
    public static final long ALARM_ISNOT_EXIST = FAULT_MODEL_BASE + LegoNumberConstant.FOUR;

    /* ********************************** 告警模块 结束 ******************************************** */
    /* ********************************** 性能模块 开始 ******************************************** */
    // 性能模块错误码相对初始值，从1792开始即0x00000700

    /**
     * PERF_MODEL_BASE
     */
    public static final long PERF_MODEL_BASE = ERROR_CODE_BASE + LegoNumberConstant.VALUE_1792;

    /* ********************************** 性能模块 结束 ******************************************** */
    /* ********************************** 分级网管开始 ******************************************** */
    // 分级模块错误码，从2048开始即0x00000700

    /**
     * NMS_MODEL_BASE
     */
    public static final long NMS_MODEL_BASE = ERROR_CODE_BASE + LegoNumberConstant.VALUE_1280;

    /**
     * FILE_NOTEXIST
     */
    public static final long FILE_NOTEXIST = NMS_MODEL_BASE + LegoNumberConstant.ELEVEN;

    /**
     * 北向网管不存在
     */
    public static final long NORTH_NMS_NOT_EXIST = NMS_MODEL_BASE + LegoNumberConstant.TWENTY_TWO;

    /* ********************************** 分级网管结束 ******************************************** */
    /* ********************************** 参数校验 **************************************** */

    /**
     * DATA_ENCRYPTION_PROTOCOL
     */
    public static final long DATA_ENCRYPTION_PROTOCOL = 1677929772L;

    /**
     * 参数错误，从RD上移至Lego中
     */
    public static final long ERR_PARAM = 1677929220L;

    /**
     * 国际化资源KEY错误码前缀
     */
    public static final String ERROR_CODE_PREFIX = "lego.err.";

    private ErrorCodeConstant() {
    }
}