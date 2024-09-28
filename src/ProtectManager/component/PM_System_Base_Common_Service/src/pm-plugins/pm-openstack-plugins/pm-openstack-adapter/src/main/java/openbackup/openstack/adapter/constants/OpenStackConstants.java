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
package openbackup.openstack.adapter.constants;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * OpenStack常量类
 *
 */
public final class OpenStackConstants {
    /**
     * 周
     */
    public static final List<String> WEEKS =
            Collections.unmodifiableList(Arrays.asList("mon", "tue", "wed", "thu", "fri", "sat", "sun"));

    /**
     * 备份任务名称
     */
    public static final String NAME = "backup_name";

    /**
     * 任务描述
     */
    public static final String DESCRIPTION = "description";

    /**
     * 备份类型
     */
    public static final String BACKUP_TYPE = "backup_type";

    /**
     * 恢复名称
     */
    public static final String RESTORE_NAME = "restore_name";

    /**
     * 恢复类型
     */
    public static final String RESTORE_TYPE = "restore_type";

    /**
     * 备份、恢复对象id
     */
    public static final String INSTANCE_ID = "instance_id";

    /**
     * 预置SLA id
     */
    public static final String GLOBAL_SLA_ID = "8556bb41-abe6-4821-870d-a0252f304dfc";

    /**
     * 项目id
     */
    public static final String PROJECT_ID = "projectId";

    private OpenStackConstants() {}
}
