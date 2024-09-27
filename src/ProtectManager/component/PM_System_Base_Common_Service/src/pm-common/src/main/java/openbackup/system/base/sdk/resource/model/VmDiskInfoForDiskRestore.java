package openbackup.system.base.sdk.resource.model;

import lombok.Getter;
import lombok.Setter;

/**
 * VMware磁盘恢复时所需要的虚拟机信息
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-12-08
 */
@Setter
@Getter
public class VmDiskInfoForDiskRestore {
    private String storages;

    private String targetStoreName;

    private String targetStoreMoRef;
}
