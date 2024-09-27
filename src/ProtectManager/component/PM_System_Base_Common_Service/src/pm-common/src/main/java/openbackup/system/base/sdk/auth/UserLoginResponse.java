package openbackup.system.base.sdk.auth;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述
 *
 * @author l00272247
 * @since 2020-12-06
 */
@Data
@Builder(toBuilder = true)
@NoArgsConstructor
@AllArgsConstructor
public class UserLoginResponse {
    private String token;

    private boolean modifyPassword;

    private long expireDay = -1L;

    private String userId;
}
