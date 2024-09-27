package openbackup.data.access.framework.protection.service.job;

import openbackup.data.access.framework.protection.common.util.JobExtendInfoUtil;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import com.huawei.oceanprotect.exercise.service.ExerciseQueryService;
import com.huawei.oceanprotect.job.dto.JobExerciseDetail;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

/**
 * UnfiedExerciseJobProvider
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-04-30
 */
@Slf4j
@Component("unifiedExerciseJobProvider")
public class UnifiedExerciseJobProvider implements JobProvider {
    @Autowired
    @Qualifier("unifiedJobProvider")
    private JobProvider unifiedJobProvider;

    @Autowired
    private ExerciseQueryService exerciseQueryService;

    @Override
    public boolean applicable(String jobType) {
        return JobTypeEnum.EXERCISE.getValue().equals(jobType);
    }

    @Override
    public void stopJob(String associativeId) {
        unifiedJobProvider.stopJob(associativeId);
    }

    @Override
    public void fillJobInfo(Job insertJob) {
        if (!VerifyUtil.isEmpty(insertJob.getExerciseId())) {
            JobExtendInfoUtil.fillJobPolicyInfo(insertJob, exerciseQueryService::queryExercise,
                ext -> insertJob.getExerciseId(), JobExerciseDetail.class, null);
        }
    }
}
