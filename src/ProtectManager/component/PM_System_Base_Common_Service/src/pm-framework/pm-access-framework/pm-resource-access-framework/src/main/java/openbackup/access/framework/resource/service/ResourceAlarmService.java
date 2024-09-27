/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.access.framework.resource.service;

import java.util.Set;

/**
 * 资源警告类
 *
 * @author y30044273
 * @since 2023-06-06
 */
public interface ResourceAlarmService {
    /**
     *
     * 删除受保护资源时告警
     *
     * @param uuids 资源uuid
     */
    void alarmDeleteProtectedResource(Set<String> uuids);
}
