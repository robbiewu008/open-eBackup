package openbackup.system.base.sdk.infrastructure.model;

import lombok.Data;

/**
 * infrastructure response
 *
 * @author t00508428
 * @since 2020-12-10
 */
@Data
public class InfrastructureResponse {
    private boolean success;

    private String code;

    private String message;
}
