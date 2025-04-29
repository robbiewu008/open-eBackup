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
#ifndef PROTECT_SERVICE_IMP_H
#define PROTECT_SERVICE_IMP_H

#ifdef WIN32
#include "define/Defines.h"
#endif

#include "ProtectService.h"

#ifdef WIN32
class AGENT_API ProtectServiceImp;
#endif

#ifndef EXTER_ATTACK
#define EXTER_ATTACK
#endif
class ProtectServiceImp : public AppProtect::ProtectServiceIf {
public:
    ProtectServiceImp();
    ~ProtectServiceImp();

    /**
     * abort running job, synchronization function
     * jobId : main job id
     * subJobId : abort sub job id, which is emtpy string when abort prerequisite, generate job
     * apptype : job application type
     *
     * @param jobId
     * @param subJobId
     * @param appType
    */
    EXTER_ATTACK virtual void AsyncAbortJob(ActionResult& returnValue, const std::string& jobId,
        const std::string& subJobId, const std::string& appType) override;

    /**
     * pause running job, synchronization function
     * jobId : main job id
     * subJobId : abort sub job id, which is emtpy string when abort prerequisite, generate job
     * apptype : job application type
     *
     * @param jobId
     * @param subJobId
     * @param appType
    */
    EXTER_ATTACK virtual void PauseJob(ActionResult& returnValue, const std::string& jobId, const std::string& subJobId,
        const std::string& appType) override;

    /**
     * check the backup job type, synchronization function,
     * .eg whether the backup job need to be covert from incremental to full
     * job: backup job information
     *
     * @param job
    */
    EXTER_ATTACK virtual void CheckBackupJobType(ActionResult& returnValue, const AppProtect::BackupJob& job) override;

    /**
     * check the backup job whether can be excuted in this node
     * job: backup job information
     * limit: backup node policy
     *
     * @param job
     * @param limit
    */
    EXTER_ATTACK virtual void AllowBackupInLocalNode(ActionResult& returnValue, const AppProtect::BackupJob& job,
        const AppProtect::BackupLimit::type limit) override;

    /**
    * Function description
    *     check the backup sub job whether can be executed in this node
    * Parameters
    *     job : backup job information
    *     subJob : sub job information, it's created in generating step
    * Return value
    *     successful if the ActionResult.code is 0, failed otherwise
    *
    * @param job
    * @param subJob
    */
    EXTER_ATTACK virtual void AllowBackupSubJobInLocalNode(
        ActionResult& returnValue, const AppProtect::BackupJob& job, const AppProtect::SubJob& subJob) override;

	/**
    * Function description
    *		Query the repositories needed to be scanned
	* Parameters
	* 		job : backup job information
    * 		ScanRepositories : return the repositories needed to be scanned.
	* Return value
    * 		return the repositories needed to be scanned.
	* @param job
	* @param scanRepositories
    */
    EXTER_ATTACK virtual void QueryScanRepositories(AppProtect::ScanRepositories& scanRepositories,
        const AppProtect::BackupJob& job);

    /**
    * Function description
    *     query job permission, synchronization function
    * Parameters
    *     appEnv : protect environment information
    *     application : protect application information
    * Return value
    *     return job permission, return empty if the nas share does not need to be modified
    *
    * @param appEnv
    * @param application
    */
    EXTER_ATTACK virtual void QueryJobPermission(AppProtect::JobPermission& returnJobPermission,
        const ApplicationEnvironment& appEnv, const Application& application) override;

    /**
     * execute backup prerequisite job, synchronization function
     * job: backup job information
     *
     * @param job
    */
    EXTER_ATTACK virtual void AsyncBackupPrerequisite(
        ActionResult& returnValue, const AppProtect::BackupJob& job) override;

    /**
     * execute backup generate job, asynchronization function, reporting job detail is required
     * job: backup job information
     * subJobId: report job details with the subJobId
     *
     * @param job
     * @param subJobId
    */
    EXTER_ATTACK virtual void AsyncBackupGenerateSubJob(
        ActionResult& returnValue, const AppProtect::BackupJob& job, const int32_t nodeNum) override;

    /**
     * execute backup job, asynchronization function, reporting job detail is required
     * job: backup job information
     * subJob: sub job information, it's created in generating step
     *
     * @param job
     * @param subJob
    */
    EXTER_ATTACK virtual void AsyncExecuteBackupSubJob(ActionResult& returnValue, const AppProtect::BackupJob& job,
        const SubJob& subJob) override;

    /**
     * execute backup post job, synchronization function
     * job: backup job information
     * backupJobResult: backup job result, according this result,
     * the plugin can execute normal post job or failed post job
     *
     * @param job
     * @param backupJobResult
    */
    EXTER_ATTACK virtual void AsyncBackupPostJob(ActionResult& returnValue, const AppProtect::BackupJob& job,
        const AppProtect::SubJob& subJob, const AppProtect::JobResult::type backupJobResult) override;

    /**
    * Function description
    *     check the restore job whether can be executed in this node.
    * Parameters
    *     job : backup job information
    * Return value
    *     successful if the ActionResult.code is 0, failed otherwise
    *
    * @param job
    */
    EXTER_ATTACK virtual void AllowRestoreInLocalNode(
        ActionResult& returnValue, const AppProtect::RestoreJob& job) override;

  /**
   * Function description
   *     check the restore sub job whether can be executed in this node.
   * Parameters
   *     job : backup job information
   *     subJob : sub job information
   * Return value
   *     successful if the ActionResult.code is 0, failed otherwise
   *
   * @param job
   * @param subJob
   */
    EXTER_ATTACK virtual void AllowRestoreSubJobInLocalNode(
        ActionResult& returnValue, const AppProtect::RestoreJob& job, const AppProtect::SubJob& subJob) override;

    /**
     * execute restore prerequisite job, synchronization function
     * job: restore job information
     *
     * @param job
    */
    EXTER_ATTACK virtual void AsyncRestorePrerequisite(
        ActionResult& returnValue, const  AppProtect::RestoreJob& job) override;

    /**
     * execute restore generate job, asynchronization function, reporting job detail is required
     * job: restore job information
     * subJobId: report job details with the subJobId
     *
     * @param job
     * @param nodeNum
    */
    EXTER_ATTACK virtual void AsyncRestoreGenerateSubJob(
        ActionResult& returnValue, const  AppProtect::RestoreJob& job, const int32_t nodeNum) override;

    /**
     * execute restore job, asynchronization function, reporting job detail is required
     * job: restore job information
     * subJob: sub job information, it's created in generating step
     *
     * @param job
     * @param subJob
    */
    EXTER_ATTACK virtual void AsyncExecuteRestoreSubJob(ActionResult& returnValue, const AppProtect::RestoreJob& job,
        const SubJob& subJob) override;

    /**
     * execute restore post job, synchronization function
     * job: restore job information
     * restoreJobResult: restore job result, according this result,
     * the plugin can execute normal post job or failed post job
     *
     * @param job
     * @param restoreJobResult
    */
    EXTER_ATTACK virtual void AsyncRestorePostJob(ActionResult& returnValue, const AppProtect::RestoreJob& job,
        const AppProtect::SubJob& subJob, const AppProtect::JobResult::type restoreJobResult) override;

    /**
     * execute livemount generate job, asynchronization function, reporting job detail is required
     * .eg livemount database cluster, need generate severval sub job
     * job: restore job information
     * subJobId: report job details with the subJobId
     *
     * @param job
     * @param subJobId
    */
    EXTER_ATTACK virtual void AsyncLivemountGenerateSubJob(
        ActionResult& returnValue, const AppProtect::LivemountJob& job, const int32_t nodeNum) override;

    /**
     * execute livemount job, asynchronization function, reporting job detail is required
     * job: restore job information
     * subJob: sub job information, it's created in generating step
     *
     * @param job
     * @param subJob
    */
    EXTER_ATTACK virtual void AsyncExecuteLivemountSubJob(
        ActionResult& returnValue, const AppProtect::LivemountJob& job, const SubJob& subJob) override;

    /**
     * execute livemount job, asynchronization function, reporting job detail is required
     * job: restore job information
     * subJob: sub job information, it's created in generating step
     *
     * @param job
     * @param nodeNum
    */
    EXTER_ATTACK virtual void AsyncCancelLivemountGenerateSubJob(ActionResult& returnValue,
        const AppProtect::CancelLivemountJob& job, const int32_t nodeNum) override;

    /**
    * execute livemount job, asynchronization function, reporting job detail is required
    * job: restore job information
    * subJob: sub job information, it's created in generating step
    *
    * @param job
    * @param subJob
    */
    EXTER_ATTACK virtual void AsyncExecuteCancelLivemountSubJob(
        ActionResult& returnValue, const AppProtect::CancelLivemountJob& job, const SubJob& subJob) override;

    /**
     * execute instant restore prerequisite job, synchronization function
     * job: restore job information
     *
     * @param job
    */
    EXTER_ATTACK virtual void AsyncInstantRestorePrerequisite(
        ActionResult& returnValue, const AppProtect::RestoreJob& job) override;

    /**
     * execute instant restore generate job, asynchronization function, reporting job detail is required
     * job: restore job information
     * subJobId: report job details with the subJobId
     *
     * @param job
     * @param subJobId
    */
    EXTER_ATTACK virtual void AsyncInstantRestoreGenerateSubJob(
        ActionResult& returnValue, const AppProtect::RestoreJob& job, const int32_t nodeNum) override;

    /**
     * execute instant restore job, asynchronization function, reporting job detail is required
     * .eg instant restore database cluster, need generate severval sub job
     * job: restore job information
     * subJob: sub job information, it's created in generating step
     *
     * @param job
     * @param subJob
    */
    EXTER_ATTACK virtual void AsyncExecuteInstantRestoreSubJob(
        ActionResult& returnValue, const AppProtect::RestoreJob& job, const SubJob& subJob) override;

    /**
     * execute instant restore post job, synchronization function
     * job: restore job information
     * restoreJobResult: restore job result, according this result,
     * the plugin can execute normal post job or failed post job
     *
     * @param job
     * @param restoreJobResult
    */
    EXTER_ATTACK virtual void AsyncInstantRestorePostJob(ActionResult& returnValue, const AppProtect::RestoreJob& job,
        const SubJob& subJob, const AppProtect::JobResult::type restoreJobResult) override;

    /**
     * execute building index generate job, asynchronization function, reporting job detail is required
     * job: restore job information
     * subJobId: report job details with the subJobId
     *
     * @param job
     * @param subJobId
    */
    EXTER_ATTACK virtual void AsyncBuildIndexGenerateSubJob(
        ActionResult& returnValue, const AppProtect::BuildIndexJob& job, const int32_t nodeNum) override;

    /**
     * execute building index job, synchronization function
     * job: build index job information
     * subJob: sub job information, it's created in generating step
     *
     * @param job
     * @param subJob
    */
    EXTER_ATTACK virtual void AsyncBuildIndexSubJob(ActionResult& returnValue,
      const AppProtect::BuildIndexJob& job, const SubJob& subJob) override;

    /**
    * Function description
    *     execute delete copy job, asynchronization function and idempotency function, reporting job detail is required
    * Parameters
    *     job: delete job information
    *     nodeNum : the proxy node number of executing sub job,
    *         the number of sub jobs to be generated can refer to this value.
    * Return value
    *     successful if the ActionResult.code is 0, failed otherwise
    *
    * @param job
    * @param nodeNum
    */
    EXTER_ATTACK virtual void AsyncDelCopyGenerateSubJob(
        ActionResult& _return, const AppProtect::DelCopyJob& job, const int32_t nodeNum) override;

    /**
    * Function description
    *     execute delete copy job, asynchronization function and idempotency function
    * Parameters
    *     job : delete job information
    *     subJob : sub job information, it's created in generating step
    * Return value
    *     successful if the ActionResult.code is 0, failed otherwise
    *
    * @param job
    * @param subJob
    */
    EXTER_ATTACK virtual void AsyncDelCopySubJob(
        ActionResult& _return, const AppProtect::DelCopyJob& job, const SubJob& subJob) override;

    /**
    * Function description
    *     execute delete copy job, asynchronization function and idempotency function
    * Parameters
    *     job : delete job information
    *     subJob : sub job information, it's created in generating step
    * Return value
    *     successful if the ActionResult.code is 0, failed otherwise
    *
    * @param job
    * @param subJob
    */
    EXTER_ATTACK virtual void AsyncCheckCopyGenerateSubJob(
        ActionResult& _return, const AppProtect::CheckCopyJob& job, const int32_t nodeNum) override;

    /**
    * Function description
    *     execute delete copy job, asynchronization function and idempotency function
    * Parameters
    *     job : delete job information
    *     subJob : sub job information, it's created in generating step
    * Return value
    *     successful if the ActionResult.code is 0, failed otherwise
    *
    * @param job
    * @param subJob
    */
    EXTER_ATTACK virtual void AsyncCheckCopySubJob(
        ActionResult& _return, const AppProtect::CheckCopyJob& job, const SubJob& subJob) override;
    /**
    * Function description
    *     deliver task status to plugin
    * Parameters
    *     status : status of the task delivered to the plugin
    *     jobId : main job id
    *     script : script path
    * Return value
    *     successful if the ActionResult.code is 0, failed otherwise
    *
    * @param status
    * @param job
    * @param script
    */
    EXTER_ATTACK virtual void DeliverTaskStatus(ActionResult& returnValue, const std::string& status,
        const std::string& jobId, const std::string& script) override;
    /**
    * Function description
    *     check the check copy job whether can be executed in this node.
    * Parameters
    *     job : check copy job information
    * Return value
    *     successful if the ActionResult.code is 0, failed otherwise
    *
    * @param job
    */
    EXTER_ATTACK virtual void AllowCheckCopyInLocalNode(ActionResult& returnValue, const AppProtect::CheckCopyJob& job);
    /**
    * Function description
    *     check the check job sub job whether can be executed in this node.
    * Parameters
    *     job : check job information
    *     subJob : sub job information
    * Return value
    *     successful if the ActionResult.code is 0, failed otherwise
    *
    * @param job
    * @param subJob
    */
    EXTER_ATTACK virtual void AllowCheckCopySubJobInLocalNode(ActionResult& returnValue,
        const AppProtect::CheckCopyJob& job, const SubJob& subJob);
    
    /**
    * Function description
    *     check the del copy job whether can be executed in this node.
    * Parameters
    *     job : del copy job information
    * Return value
    *     successful if the ActionResult.code is 0, failed otherwise
    *
    * @param job
    */
    EXTER_ATTACK virtual void AllowDelCopyInLocalNode(ActionResult& returnValue, const AppProtect::DelCopyJob& job);
    /**
    * Function description
    *     check the del job sub job whether can be executed in this node.
    * Parameters
    *     job : del job information
    *     subJob : sub job information
    * Return value
    *     successful if the ActionResult.code is 0, failed otherwise
    *
    * @param job
    * @param subJob
    */
    EXTER_ATTACK virtual void AllowDelCopySubJobInLocalNode(ActionResult& returnValue,
        const AppProtect::DelCopyJob& job, const SubJob& subJob);
};
#endif // _PROTECT_SERVICE_IMP_H_