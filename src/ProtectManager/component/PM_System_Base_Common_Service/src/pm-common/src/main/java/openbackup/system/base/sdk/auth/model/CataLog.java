package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * CataLog
 *
 * @author l30044826
 * @since 2023-07-07
 */
@Getter
@Setter
public class CataLog {
    private String type;

    private String id;

    private String name;

    private List<HcsEndpoint> endpoints;
}
