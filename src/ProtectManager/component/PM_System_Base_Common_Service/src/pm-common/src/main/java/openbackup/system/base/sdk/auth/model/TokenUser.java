package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

/**
 * TokenUser
 *
 * @author l30044826
 * @since 2023-07-07
 */
@Getter
@Setter
public class TokenUser {
    private Domain domain;

    private String id;

    private String name;

    private String passwordExpiresAt;
}
