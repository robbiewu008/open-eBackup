package openbackup.system.base.sdk.cluster.model;

import lombok.Data;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

/**
 * Modify dorado passwd request
 *
 * @author p30001902
 * @since 2020-12-02
 */
@Data
public class StorageModifyPwdRequest {
    private static final int MIN_ACCOUNT_LEN = 1;

    private static final int MAX_ACCOUNT_LEN = 256;

    // 用户ID，必选
    @NotNull
    @Length(min = MIN_ACCOUNT_LEN, max = MAX_ACCOUNT_LEN)
    private String userId;

    // 新密码，必选
    @NotNull
    @Length(min = MIN_ACCOUNT_LEN, max = MAX_ACCOUNT_LEN)
    private String newPassword;

    // 存储的esn
    @Size(min = 1, max = 256)
    private String esn;
}
