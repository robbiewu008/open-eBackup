package openbackup.data.access.framework.protection.handler.v1.replication;

import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.util.Applicable;

import java.util.concurrent.atomic.AtomicStampedReference;

/**
 * Replication Copy Processor
 *
 * @author l00272247
 * @since 2020-12-18
 */
public interface ReplicationCopyProcessor extends Applicable<String> {
    /**
     * process replication copy
     *
     * @param taskCompleteMessage task complete message
     * @return replicated copy number
     */
    AtomicStampedReference<Boolean> process(TaskCompleteMessageBo taskCompleteMessage);
}
