/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.common.constants;

import lombok.Getter;

/**
 * 权限操作枚举类
 *
 * @author z00842230
 * @since 2024-07-08
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
     * 即时挂载策略
     */
    LIVE_MOUNT_POLICY("liveMountPolicy");

    private String authOperation;

    AuthOperationEnum(String authOperation) {
        this.authOperation = authOperation;
    }
}
