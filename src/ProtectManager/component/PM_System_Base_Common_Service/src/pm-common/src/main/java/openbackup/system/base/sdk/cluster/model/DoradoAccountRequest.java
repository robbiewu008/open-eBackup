package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.sdk.cluster.enums.ClusterEnum;

import lombok.Data;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotNull;

/**
 * Dorado account request
 *
 * @author p30001902
 * @since 2020-12-02
 */
@Data
public class DoradoAccountRequest {
    private static final int MIN_ACCOUNT_LEN = 1;

    private static final int MAX_ACCOUNT_LEN = 256;

    @NotNull
    @Length(min = MIN_ACCOUNT_LEN, max = MAX_ACCOUNT_LEN)
    private String userName;

    @Length(max = MAX_ACCOUNT_LEN)
    private String description;

    @NotNull
    @Length(min = MIN_ACCOUNT_LEN, max = MAX_ACCOUNT_LEN)
    private String password;

    private ClusterEnum.DoradoCreateUserType userType;

    @Length(min = MIN_ACCOUNT_LEN, max = MAX_ACCOUNT_LEN)
    private String esn;
}
