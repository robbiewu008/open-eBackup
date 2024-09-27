/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.system;

import openbackup.system.base.sdk.system.model.TimeZoneInfo;

/**
 * 系统上下文服务接口定义，提供系统相关上下文信息的获取接口定义
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-21
 */
public interface SystemContextService {
    /**
     * 获取系统的语言
     *
     * @return 返回语言
     */
    String getSystemLanguage();

    /**
     * 查询DM系统时区信息
     *
     * @return 系统时区信息
     */
    TimeZoneInfo getSystemTimeZone();
}
