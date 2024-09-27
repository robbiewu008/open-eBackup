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
 * 演练任务label常量类
 *
 * @author w30044259
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-11-14
 */
public class ExerciseJobLabels {
    /**
     * 演练任务开始执行
     */
    public static final String EXERCISE_START_JOB_LABEL = "exercise_start_execute";

    /**
     * 资源（{0}）创建演练子任务（{1}）成功，子任务id（{2}）。参数：资源id、子任务类型，子任务id
     */
    public static final String EXERCISE_CREATE_SUB_JOB_SUCCESS = "exercise_create_sub_job_success";

    /**
     * 资源（{0}）创建演练子任务（{1}）失败。参数：资源id、子任务类型
     */
    public static final String EXERCISE_CREATE_SUB_JOB_FAIL = "exercise_create_sub_job_fail";

    /**
     * 演练任务执行失败
     */
    public static final String EXERCISE_JOB_FAIL_LABEL = "exercise_job_fail_label";

    /**
     * 演练子任务类型即时挂载
     */
    public static final String EXERCISE_LIVE_MOUNT = "agent_task_type_mount";

    /**
     * 演练子任务类型恢复
     */
    public static final String EXERCISE_RESTORE = "agent_task_type_restore";

    /**
     * 演练子任务类型挂载销毁
     */
    public static final String EXERCISE_UNMOUNT = "agent_task_type_unmount";
}
