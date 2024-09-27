package openbackup.data.protection.access.provider.sdk.archive;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.job.Task;

import com.fasterxml.jackson.core.JsonProcessingException;

/**
 * This interface defines the restore providers that need to be implemented by DataMover
 *
 * @author y00490893
 * @version [OceanStor 100P 8.1.0]
 * @since 2020-12-12
 */
public interface ArchiveProvider extends DataProtectionProvider<String> {
    /**
     * restore methods that need to be implemented by specific providers, This method is responsible for the business
     * logic of copy restore.
     *
     * @param archiveObject the restore object
     * @return the restore task
     */
    Task archive(ArchiveObject archiveObject);

    /**
     * 生成恢复任务
     *
     * @param archiveRequest 恢复请求
     * @return 任务json
     * @throws JsonProcessingException json转换异常
     */
    String createArchiveTask(ArchiveRequest archiveRequest) throws JsonProcessingException;
}
