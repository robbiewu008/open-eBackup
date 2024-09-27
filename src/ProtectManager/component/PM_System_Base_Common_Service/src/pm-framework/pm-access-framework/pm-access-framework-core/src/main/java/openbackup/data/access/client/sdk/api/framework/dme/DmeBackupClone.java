package openbackup.data.access.client.sdk.api.framework.dme;

import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;

import lombok.Data;

import java.util.List;

/**
 * Dme Backup Clone
 *
 * @author l00272247
 * @since 2021-12-28
 */
@Data
public class DmeBackupClone {
    private String srcCopyId;

    private String targetCopyId;

    // 0表示可写，1表示只读
    private int type;

    private String fileSystemName;

    // 针对远端存储设备创建克隆，在PM下发任务时，携带repo信息
    private String snapShotName;

    private List<StorageRepository> repositories;
}
