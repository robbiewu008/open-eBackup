package openbackup.system.base.sdk.license.module;

import lombok.Data;

/**
 * 功能描述
 *
 * @author s00455050
 * @since 2021-01-29
 */
@Data
public class DoradoLicenseResponse<T> {
    private T data;

    private DoradoLicenseError error;
}
