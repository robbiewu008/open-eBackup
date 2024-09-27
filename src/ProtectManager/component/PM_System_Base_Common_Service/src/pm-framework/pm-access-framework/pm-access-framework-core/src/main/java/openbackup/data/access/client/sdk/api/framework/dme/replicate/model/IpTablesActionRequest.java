package openbackup.data.access.client.sdk.api.framework.dme.replicate.model;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述
 *
 * @author y30037959
 * @since 2022-11-02
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class IpTablesActionRequest {
    /**
     * ip
     */
    private String ip;

    /**
     * 端口
     */
    private String port;

    /**
     * add, query， delete
     **/
    private String actionType;
}
