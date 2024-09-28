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
package openbackup.data.access.framework.core.common.constants;

/**
 * Kafka message topic
 *
 */
public final class TopicConstants {
    /**
     * Executing a Backup Plan
     */
    public static final String EXECUTE_BACKUP_PLAN = "protection.backup";

    /**
     * REPLICATION_TOPIC
     */
    public static final String REPLICATION_TOPIC = "protection.replication";

    /**
     * REPLICATION_COMPLETE_TOPIC
     */
    public static final String REPLICATION_COMPLETE_TOPIC = "replication.complete";


    /**
     * Gen Index
     */
    public static final String GEN_INDEX = "copy.save.event";

    /**
     * 恢复topic
     */
    public static final String EXECUTE_RESTORE = "protection.restore";

    /**
     * Executing a Plan Fail
     */
    public static final String EXECUTE_BACKUP_DONE = "protection.backup.done";

    /**
     * Gen Index Donec
     */
    public static final String GEN_INDEX_DONE = "IndexRequest";

    /**
     * Gen Index Done
     */
    public static final String INDEX_RESPONSE = "IndexResponse";

    /**
     * Delete Index Request
     */
    public static final String DELETE_INDEX_REQUEST = "IndexDeleteRequest";

    /**
     * Delete Index Response
     */
    public static final String DELETE_INDEX_RESPONSE = "IndexDeleteResponse";

    /**
     * Scan request
     */
    public static final String SCAN_REQUEST = "ScanRequest";

    /**
     * Scan response
     */
    public static final String SCAN_RESPONSE = "ScanResponse";


    /**
     * Copy Delete Task finished
     */
    public static final String COPY_DELETE_JOB_MONITOR_FINISHED = "copy.delete.job.monitor.finished";

    /**
     * topic of stopping job
     */
    public static final String JOB_STOP_TOPIC = "Job.stop.task";

    /**
     * recheck copy is expire
     */
    public static final String COPY_CHECK_REQUEST = "copy.check.request";

    /**
     * topic when task complete
     */
    public static final String TASK_COMPLETE_TOPIC = "TaskCompleteMessage";

    /**
     * task fail topic
     */
    public static final String TASK_FAIL_TOPIC = "TaskFailMessage";

    /**
     * topic when task complete
     */
    public static final String JOB_COMPLETE_TOPIC = "job.complete";

    /**
     * topic when update log
     */
    public static final String JOB_UPDATE_TOPIC = "job.log.update";

    /**
     * task success topic
     */
    public static final String TASK_RESTORE_DONE_TOPIC = "protection.restore.done";

    /**
     * protection archiving topic
     */
    public static final String PROTECTION_ARCHIVE = "protection.archive";

    /**
     * REPLICATION_COMPLETE
     */
    public static final String REPLICATION_COMPLETE = "replication.complete";

    /**
     * 副本复制完成
     */
    public static final String REPLICATION_COMPLETED_TOPIC = "copy.replication.completed";

    /**
     * TASK_ARCHIVE_DONE_TOPIC
     */
    public static final String TASK_ARCHIVE_DONE_TOPIC = "protection.archive.done";

    /**
     * REPLICATION_REMOVEPAIR
     */
    public static final String PROTECTION_REMOVE_EVENT = "protection.remove.event";

    /**
     * PROTECTION_CHANGE_EVENT
     */
    public static final String PROTECTION_CHANGE_EVENT = "protection.change.event";

    /**
     * SLA_CHANGED_EVENT
     */
    public static final String SLA_CHANGED_EVENT = "SlaChangedRequest";

    /**
     * REPLICATION_REMOVEPAIR
     */
    public static final String FLR_TOPIC = "FilesRestoreRequest";

    /**
     * KAFKA_CONSUMER_GROUP
     */
    public static final String KAFKA_CONSUMER_GROUP = "consumerGroup";

    /**
     * KAFKA_CONSUMER_ERROR_HANDLER
     */
    public static final String KAFKA_CONSUMER_ERROR_HANDLER = "consumerAwareErrorHandler";

    /**
     * archive copies to s3
     */
    public static final String ARCHIVE_COPIES_TO_S3_TOPIC = "archive.copies.to.repository";

    /**
     * Import archive copies
     */
    public static final String IMPORT_ARCHIVE_COPIES_TOPIC = "import.archive.copies";

    /**
     * Copies delete
     */
    public static final String COPY_DELETE_TOPIC = "copy.delete";

    /**
     * force job stop
     */
    public static final String FORCE_JOB_STOP = "job.force.stop";

    /**
     *  backup success
     */
    public static final String PROTECTION_BACKUP_SUCCESS = "protection.backup.success";

    /**
     *  backup complete
     */
    public static final String PROTECTION_BACKUP_COMPLETE = "protection.backup.completed";

    /**
     * 恢复执行V2版本topic
     */
    public static final String RESTORE_EXECUTE_V2 = "protection.restore.execute.v2";

    /**
     * 恢复V2版本topic对应的消费组
     */
    public static final String RESTORE_GROUP_V2 = "restoreGroup";

    /**
     * 执行副本校验任务topic
     */
    public static final String COPY_VERIFY_EXECUTE = "copyVerifyExecute";

    /**
     * 执行副本校验消费组
     */
    public static final String COPY_VERIFY_GROUP = "copyVerifyGroup";

    /**
     * 智能侦测任务topic
     */
    public static final String INTELLIGENT_DETECTION_EXECUTE = "intelligentDetectionExecute";

    /**
     * 智能侦测消费组
     */
    public static final String INTELLIGENT_DETECTION_GROUP = "intelligentDetectionExecuteGroup";

    /**
     * 演练任务初始化
     */
    public static final String EXERCISE_START = "exercise.start";

    /**
     * archive after detection
     */
    public static final String ARCHIVE_AFTER_DETECTION = "archive.copies.detection.complete";

    private TopicConstants() {
    }
}
