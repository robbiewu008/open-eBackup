package openbackup.data.protection.access.provider.sdk.archive;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述
 *
 * @author z30009433
 * @since 2020-12-30
 */
@Builder
@AllArgsConstructor
@NoArgsConstructor
@Data
public class ArchiveImportObject {
    private String storageId;

    private String requestId;

    private String archiveCopyId;

    private String backupCopyId;

    private int type;

    private String policy;

    private String jobId;

    private int autoRetryTimes;
}
