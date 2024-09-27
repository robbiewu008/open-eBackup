package openbackup.data.access.framework.protection.service.replication;

import openbackup.data.access.framework.protection.handler.v1.replication.ReplicationCopyProcessor;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.concurrent.atomic.AtomicStampedReference;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Advance Replication Copy Processor
 *
 * @author l00272247
 * @since 2020-12-30
 */
@Component
public class UnifiedReplicationCopyProcessor implements ReplicationCopyProcessor {
    private static final List<String> SUPPORTED_RESOURCES =
        Stream.of(ResourceSubTypeEnum.ORACLE, ResourceSubTypeEnum.VMWARE, ResourceSubTypeEnum.IMPORT_COPY)
            .map(ResourceSubTypeEnum::getType)
            .collect(Collectors.toList());

    /**
     * process replication copy
     *
     * @param taskCompleteMessage task complete message
     * @return replicated copy number
     */
    @Override
    public AtomicStampedReference<Boolean> process(TaskCompleteMessageBo taskCompleteMessage) {
        JSONArray backupCopyList = JSONArray.fromObject(taskCompleteMessage.getExtendsInfo().get("backup_copy_list"));
        int count;
        if (taskCompleteMessage.getJobStatus() != IsmNumberConstant.SIX && !VerifyUtil.isEmpty(backupCopyList)) {
            count = backupCopyList.size();
        } else {
            count = 0;
        }
        return new AtomicStampedReference<>(true, count);
    }

    @Override
    public boolean applicable(String object) {
        return SUPPORTED_RESOURCES.contains(object);
    }
}
