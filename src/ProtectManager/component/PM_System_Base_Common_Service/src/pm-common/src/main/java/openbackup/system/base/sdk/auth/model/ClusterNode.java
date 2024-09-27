package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

/**
 * ClusterNode
 *
 * @author y30021475
 * @since 2023-08-02
 */
@Getter
@Setter
public class ClusterNode {
    private String name;

    private String ipAddresses;
}
