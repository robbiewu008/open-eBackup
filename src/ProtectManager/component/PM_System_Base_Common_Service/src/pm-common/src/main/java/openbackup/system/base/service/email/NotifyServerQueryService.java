/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.service.email;

import openbackup.system.base.service.email.entity.RemoteNotifyServer;

/**
 * 邮箱服务器查询服务
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-14
 */
public interface NotifyServerQueryService {
    /**
     * 获取邮箱服务器
     *
     * @return RemoteNotifyServer
     */
    RemoteNotifyServer queryRemoteNotifyServer();
}
