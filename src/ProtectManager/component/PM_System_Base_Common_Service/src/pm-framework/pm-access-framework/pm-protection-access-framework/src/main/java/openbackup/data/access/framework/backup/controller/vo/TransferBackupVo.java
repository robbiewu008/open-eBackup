package openbackup.data.access.framework.backup.controller.vo;

import lombok.Getter;
import lombok.Setter;

import javax.validation.constraints.NotNull;

/**
 * 转换备份类型结构体
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/5/27
 */
@Getter
@Setter
public class TransferBackupVo {
    @NotNull
    private String backupType;

    @NotNull
    private String resourceId;
}
