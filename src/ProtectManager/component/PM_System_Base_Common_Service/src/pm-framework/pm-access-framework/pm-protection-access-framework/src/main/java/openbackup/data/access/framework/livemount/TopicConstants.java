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
package openbackup.data.access.framework.livemount;

/**
 * Topic Constants
 *
 */
public class TopicConstants {
    /**
     * LIVE_MOUNT_EXECUTE_REQUEST
     */
    public static final String LIVE_MOUNT_EXECUTE_REQUEST = "live-mount.execute.request";

    /**
     * LIVE_MOUNT_UNMOUNT_BEFORE_EXECUTE
     */
    public static final String LIVE_MOUNT_UNMOUNT_BEFORE_EXECUTE = "live-mount.unmount-before-execute";

    /**
     * LIVE_MOUNT_UNMOUNTED_BEFORE_EXECUTE_PROCESS
     */
    public static final String LIVE_MOUNT_UNMOUNTED_BEFORE_EXECUTE_PROCESS
        = "live-mount.unmounted-before-execute.process";

    /**
     * LIVE_MOUNT_UNMOUNTED_BEFORE_EXECUTE_PROCESS
     */
    public static final String LIVE_MOUNT_UNMOUNTED_BEFORE_EXECUTE_FAILED
        = "live-mount.unmounted-before-execute.fail";

    /**
     * LIVE_MOUNT_OLD_COPY_CLEANED_PROCESS
     */
    public static final String LIVE_MOUNT_OLD_COPY_CLEANED_PROCESS = "live-mount.old-copy-cleaned.process";

    /**
     * LIVE_MOUNT_COPY_CLONE
     */
    public static final String LIVE_MOUNT_COPY_CLONE = "live-mount.copy.clone";

    /**
     * LIVE_MOUNT_EXECUTE_PROCESS
     */
    public static final String LIVE_MOUNT_EXECUTE_PROCESS = "live-mount.execute.process";

    /**
     * LIVE_MOUNT_EXECUTE_PROCESS
     */
    public static final String LIVE_MOUNT_EXECUTE_PROCESS_WITHOUT_CLONE_COPY
        = "live-mount.execute.process.without.clone.copy";

    /**
     * LIVE_MOUNT_COMPLETE_PROCESS
     */
    public static final String LIVE_MOUNT_COMPLETE_PROCESS = "live-mount.complete.process";

    /**
     * LIVE_MOUNT_UNMOUNT_REQUEST
     */
    public static final String LIVE_MOUNT_UNMOUNT_REQUEST = "live-mount.unmount.request";

    /**
     * LIVE_MOUNT_UNMOUNT_ACTION
     */
    public static final String LIVE_MOUNT_UNMOUNT_ACTION = "live-mount.unmount.action";

    /**
     * LIVE_MOUNT_SCHEDULE
     */
    public static final String LIVE_MOUNT_SCHEDULE = "live-mount.schedule";

    /**
     * LIVE_MOUNT_UNMOUNT_REQUEST
     */
    public static final String LIVE_MOUNT_UNMOUNT_COMPLETE = "live-mount.unmount.complete";

    /**
     * LIVE_MOUNT_CLEAN_CLONE_COPY
     */
    public static final String LIVE_MOUNT_CLEAN_CLONE_COPY = "live-mount.clone-copy.clean";

    /**
     * LIVE_MOUNT_UNMOUNT_REQUEST
     */
    public static final String LIVE_MOUNT_MIGRATE_REQUEST = "live-mount.migrate.request";

    /**
     * LIVE_MOUNT_UNMOUNT_REQUEST
     */
    public static final String LIVE_MOUNT_MIGRATE_COMPLETE = "live-mount.migrate.complete";
}
