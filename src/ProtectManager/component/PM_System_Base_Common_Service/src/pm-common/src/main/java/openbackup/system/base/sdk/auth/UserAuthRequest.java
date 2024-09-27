package openbackup.system.base.sdk.auth;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述
 *
 * @author p30001902
 * @since 2020-09-04
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class UserAuthRequest {
    private String userName;

    private String password;
}
