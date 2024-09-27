package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.Locale;

/**
 * 资源扩展字段参数
 *
 * @author z30027603
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-04-04
 */
@Getter
@Setter
public class NextBackupParams {
    /**
     * 下次备份类型。在保护对象ext的key
     *
     * 值为 BackupTypeEnum的小写。 例如 full
     */
    @JsonProperty("next_backup_type")
    private String nextBackupType;

    /**
     * 下次备份类型引发原因。在保护对象ext的key
     *
     * 值为 NextBackupChangeCauseEnum。 例如 LOG_BACKUP_SUCCESS
     */
    @JsonProperty("next_backup_change_cause")
    private String nextBackupChangeCause;

    public NextBackupParams(String nextBackupChangeCause) {
        this(BackupTypeEnum.FULL.name().toLowerCase(Locale.ROOT), nextBackupChangeCause);
    }

    public NextBackupParams(String nextBackupType, String nextBackupChangeCause) {
        this.nextBackupType = nextBackupType;
        this.nextBackupChangeCause = nextBackupChangeCause;
    }
}
