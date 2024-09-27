package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Data;

/**
 * The GetClusterEsnReq
 *
 * @author b30042790
 * @since 2023-06-16
 */
@Data
public class GetClusterEsnReq {
    /**
     * 集群esn
     */
    private String esn;
}
