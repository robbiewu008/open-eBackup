package openbackup.system.base.sdk.auth.model.request;

import lombok.Getter;
import lombok.Setter;

/**
 * BasePasswds
 *
 * @author y30021475
 * @since 2023-09-13
 */
@Getter
@Setter
public class BasePasswds {
    private String component;

    private String regionId;

    private String accountName;
}
