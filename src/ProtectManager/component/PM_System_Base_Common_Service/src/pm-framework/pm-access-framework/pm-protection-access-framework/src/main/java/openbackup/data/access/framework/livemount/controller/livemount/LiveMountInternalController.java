package openbackup.data.access.framework.livemount.controller.livemount;

import openbackup.data.access.framework.livemount.service.LiveMountService;
import openbackup.system.base.common.annotation.JobScheduleControl;
import openbackup.system.base.common.annotation.JobScheduleControls;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.JobBo;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * Live Mount Policy Internal Controller
 *
 * @author m00576658
 * @since 2021-03-09
 */
@RestController
@RequestMapping("v1/internal/live-mount")
@JobScheduleControls({
    @JobScheduleControl(
            jobType = JobTypeEnum.LIVE_MOUNT,
            majorPriority = IsmNumberConstant.THOUSAND,
            globalJobLimit = "{'common': 'RUNNING_JOB_LIMIT_COUNT_ONE_NODE'}",
            scope =
                    "[{'queue_job_type':"
                        + " ['BACKUP','RESTORE','live_mount','COPY_DELETE','unmount']},'root_uuid']",
            scopeJobLimit = -1),
    @JobScheduleControl(
            jobType = JobTypeEnum.UNMOUNT,
            majorPriority = IsmNumberConstant.THOUSAND,
            globalJobLimit = "{'common': 'RUNNING_JOB_LIMIT_COUNT_ONE_NODE'}",
            scope =
                    "[{'queue_job_type':"
                        + " ['BACKUP','RESTORE','live_mount','COPY_DELETE','unmount']},'root_uuid']",
            scopeJobLimit = -1),
    @JobScheduleControl(
            jobType = JobTypeEnum.MIGRATE,
            majorPriority = IsmNumberConstant.THOUSAND,
            globalJobLimit = "{'common': 'RUNNING_JOB_LIMIT_COUNT_ONE_NODE'}"),
    @JobScheduleControl(
            jobType = JobTypeEnum.HOST_REGISTER,
            majorPriority = IsmNumberConstant.THOUSAND,
            globalJobLimit = "{'host_register': 'HOST_REGISTER_JOB_LIMIT_COUNT_ONE_NODE'}"),
    @JobScheduleControl(
            jobType = JobTypeEnum.PROTECT_AGENT_UPDATE,
            majorPriority = IsmNumberConstant.THOUSAND,
            globalJobLimit = "{'host_register': 'HOST_REGISTER_JOB_LIMIT_COUNT_ONE_NODE'}"),
    @JobScheduleControl(
        jobType = JobTypeEnum.ANTI_RANSOMWARE,
        majorPriority = IsmNumberConstant.THOUSAND,
        globalJobLimit = "{'anti_ransomware': 'ANTI_RANSOMWARE_JOB_LIMIT_COUNT_ONE_NODE'}",
        scope = "['resourceId']",
        scopeJobLimit = -1),
    @JobScheduleControl(
        jobType = JobTypeEnum.PROTECT_AGENT_UPDATE_PLUGIN_TYPE,
        majorPriority = IsmNumberConstant.THOUSAND,
        globalJobLimit = "{'host_register': 'HOST_REGISTER_JOB_LIMIT_COUNT_ONE_NODE'}")
})
@Slf4j
public class LiveMountInternalController {
    private static final String CALLBACK_CANCEL_LIVE_MOUNT_ID = "callback.cancel.live_mount_id";

    @Autowired
    LiveMountService liveMountService;

    /**
     * reset live mount user id
     *
     * @param userId user id
     */
    @ExterAttack
    @PutMapping("/action/revoke/{user_id}")
    public void revokeLiveMountUserId(@PathVariable("user_id") String userId) {
        if (VerifyUtil.isEmpty(userId)) {
            return;
        }
        liveMountService.revokeLiveMountUserId(userId);
    }

    /**
     * cancel job
     *
     * @param jobBo job
     */
    @ExterAttack
    @PutMapping("/action/cancel")
    public void cancelJob(@RequestBody JobBo jobBo) {
        if (!VerifyUtil.isEmpty(jobBo.getCopyId())) {
            log.info("cancel live mount fail. jobJsonString copy id is null");
        }
        String liveMountId = JSONObject.fromObject(jobBo.getData()).getString(CALLBACK_CANCEL_LIVE_MOUNT_ID);
        if (VerifyUtil.isEmpty(liveMountId)) {
            log.info("cancel live mount fail. jobJsonString data without live mount id");
        }
        liveMountService.cancelLiveMount(liveMountId);
    }
}
