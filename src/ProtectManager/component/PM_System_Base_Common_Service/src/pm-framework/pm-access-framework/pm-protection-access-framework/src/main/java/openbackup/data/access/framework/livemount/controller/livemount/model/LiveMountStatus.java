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
package openbackup.data.access.framework.livemount.controller.livemount.model;

import openbackup.data.access.framework.livemount.common.LiveMountOperateType;
import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Function;

/**
 * Live Mount Status
 *
 */
public enum LiveMountStatus {
    /**
     * NORMAL
     */
    READY("ready"),

    /**
     * MOUNTING
     */
    MOUNTING("mounting"),

    /**
     * UNMOUNTING
     */
    UNMOUNTING("unmounting"),

    /**
     * MIGRATING
     */
    MIGRATING("migrating"),

    /**
     * AVAILABLE
     */
    AVAILABLE("available"),

    /**
     * MOUNT_FAILED
     */
    MOUNT_FAILED("mount_failed"),

    /**
     * INVALID_MOUNT
     */
    INVALID("invalid");

    private String name;

    private static final List<LiveMountStatus> CAN_MOUNT_LIST = Arrays.asList(LiveMountStatus.AVAILABLE,
        LiveMountStatus.MOUNT_FAILED);

    private static final List<LiveMountStatus> CAN_DESTROY_LIST = Arrays.asList(LiveMountStatus.AVAILABLE,
        LiveMountStatus.MOUNT_FAILED, LiveMountStatus.INVALID);

    private static final List<LiveMountStatus> CAN_ACTIVATE_LIST = Arrays.asList(LiveMountStatus.AVAILABLE,
        LiveMountStatus.MOUNT_FAILED);

    private static final List<LiveMountStatus> CAN_MIGRATE_LIST = Collections.singletonList(LiveMountStatus.AVAILABLE);

    /**
     * check live mount status can do what operate
     */
    public static final Map<String, Function<String, Boolean>> STATUS_MAP = new HashMap<>();

    static {
        STATUS_MAP.put(LiveMountOperateType.UPDATE, LiveMountStatus::canMount);
        STATUS_MAP.put(LiveMountOperateType.MODIFY, LiveMountStatus::canMount);
        STATUS_MAP.put(LiveMountOperateType.MIGRATE, LiveMountStatus::canMigrate);
        STATUS_MAP.put(LiveMountOperateType.ACTIVATE, LiveMountStatus::canActivate);
        STATUS_MAP.put(LiveMountOperateType.DEACTIVATE, LiveMountStatus::canActivate);
        STATUS_MAP.put(LiveMountOperateType.DESTROY, LiveMountStatus::canDestroy);
    }

    LiveMountStatus(String name) {
        this.name = name;
    }

    @JsonValue
    public String getName() {
        return name;
    }

    /**
     * get enum by name
     *
     * @param name name
     * @return enum
     */
    @JsonCreator
    public static LiveMountStatus get(String name) {
        return EnumUtil.get(LiveMountStatus.class, LiveMountStatus::getName, name);
    }

    /**
     * check can update
     *
     * @param value value
     * @return result
     */
    public static boolean canMount(String value) {
        LiveMountStatus status = get(value);
        return CAN_MOUNT_LIST.contains(status);
    }

    /**
     * check can destroy
     *
     * @param value value
     * @return result
     */
    public static boolean canDestroy(String value) {
        LiveMountStatus status = get(value);
        return CAN_DESTROY_LIST.contains(status);
    }

    /**
     * check can migrate
     *
     * @param value value
     * @return result
     */
    public static boolean canMigrate(String value) {
        LiveMountStatus status = get(value);
        return CAN_MIGRATE_LIST.contains(status);
    }

    /**
     * check can activate
     *
     * @param value value
     * @return result
     */
    public static boolean canActivate(String value) {
        LiveMountStatus status = get(value);
        return CAN_ACTIVATE_LIST.contains(status);
    }
}
