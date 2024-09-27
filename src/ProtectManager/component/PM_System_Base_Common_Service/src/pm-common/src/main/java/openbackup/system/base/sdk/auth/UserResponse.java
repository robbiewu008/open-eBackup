package openbackup.system.base.sdk.auth;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 功能描述
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2019-11-26
 */
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
public class UserResponse extends UserInnerResponse {
    private boolean login;
}
