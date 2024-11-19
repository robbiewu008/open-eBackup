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

import lombok.Getter;

/**
 * 权限操作枚举类
 *
 */
@Getter
public enum AuthOperationEnum {
    /**
     * 管理客户端
     */
    MANAGE_CLIENT("manageClient"),
    /**
     * 管理生产资源(组)
     */
    MANAGE_RESOURCE("manageResource"),
    /**
     * 备份
     */
    BACKUP("backup"),
    /**
     * 复制
     */
    REPLICATION("replication"),
    /**
     * 归档
     */
    ARCHIVE("archive"),
    /**
     * 管理sla
     */
    SLA("sla"),
    /**
     * 管理限速策略
     */
    SPEED_LIMIT_STRATEGY("speedLimitStrategy"),
    /**
     * 副本删除
     */
    COPY_DELETE("copyDelete"),
    /**
     * 副本索引
     */
    COPY_INDEX("copyIndex"),
    /**
     * 原位置恢复
     */
    ORIGINAL_RESTORE("originalRestore"),
    /**
     * 新位置恢复
     */
    NEW_RESTORE("newRestore"),
    /**
     * 本机恢复
     */
    NATIVE_RESTORE("nativeRestore"),
    /**
     * 恢复演练
     */
    RESTORE_EXERCISE("restoreExercise"),
    /**
     * 管理air gap
     */
    AIR_GAP("airGap"),
    /**
     * 即时挂载
     */
    LIVE_MOUNT("liveMount"),
    /**
     * 管理防勒索 & worm
     */
    PREVENT_EXTORTION_AND_WORM("preventExtortionAndWorm"),
    /**
     * 脱敏
     */
    DESENSITIZATION("desensitization"),
    /**
     * 报表
     */
    REPORT("report"),
    /**
     * 报表订阅
     */
    SCHEDULE_REPORT("scheduleReport"),
    /**
     * 即时挂载策略
     */
    LIVE_MOUNT_POLICY("liveMountPolicy");

    private String authOperation;

    AuthOperationEnum(String authOperation) {
        this.authOperation = authOperation;
    }
}
