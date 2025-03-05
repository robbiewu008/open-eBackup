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
package openbackup.data.protection.access.provider.sdk.retry;

/**
 * JobMarkStatusEnum
 *
 */
public class JobRetryEnum {
    /**
     * enum markStatus of job
     *
     */
    public enum JobMarkStatusEnum {
        /**
         * not support to change markStatus of job
         */
        NOT_SUPPORT("0"),

        /**
         * the job is not marked
         */
        NOT_MARK("1"),

        /**
         * the job is marked
         */
        MARKED("2"),

        /**
         * the job is marked and retried
         */
        RETRYED("3");

        private final String status;

        /**
         * 构造方法
         *
         * @param status 标记状态
         */
        JobMarkStatusEnum(String status) {
            this.status = status;
        }

        /**
         * 获取具体标记状态值方法
         *
         * @return 标记状态值
         */
        public String getStatus() {
            return status;
        }
    }

    /**
     * retry task type of job, value of 'task_type' in 'data' of job detail
     *
     */
    public enum JobRetryTaskTypeEnum {
        /**
         * restore v1 恢复任务 v1
         */
        RESTORE_V1("restore_v1"),

        /**
         * restore v2 恢复任务 v2
         */
        RESTORE_V2("restore_v2");

        private final String type;

        JobRetryTaskTypeEnum(String type) {
            this.type = type;
        }

        /**
         * 获取任务类型
         *
         * @return 任务类型
         */
        public String getType() {
            return type;
        }
    }
}
