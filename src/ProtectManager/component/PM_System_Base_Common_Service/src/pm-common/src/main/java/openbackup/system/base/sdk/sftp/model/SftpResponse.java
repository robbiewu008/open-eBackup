package openbackup.system.base.sdk.sftp.model;

import lombok.Data;

/**
 * sftp response
 *
 * @author dWX1009286
 * @since 2021-06-10
 */
@Data
public class SftpResponse {
    private boolean isSuccess;

    private String code;

    private String message;
}
