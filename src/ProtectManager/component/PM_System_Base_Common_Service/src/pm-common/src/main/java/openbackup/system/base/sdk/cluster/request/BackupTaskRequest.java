package openbackup.system.base.sdk.cluster.request;

import openbackup.system.base.common.constants.IsmNumberConstant;

import lombok.Data;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-07-22
 */
@Data
public class BackupTaskRequest {
    @NotNull
    private Long imagesId;

    @NotNull
    @Size(max = 256)
    private String desc;

    @NotNull
    @Min(value = IsmNumberConstant.ZERO)
    @Max(value = IsmNumberConstant.ONE)
    private int backupType;

    @Size(max = 16)
    private String password;

    @NotNull
    @Size(max = 1024)
    @Pattern(regexp = "^\\d{13}$")
    private String backupPath;
}
