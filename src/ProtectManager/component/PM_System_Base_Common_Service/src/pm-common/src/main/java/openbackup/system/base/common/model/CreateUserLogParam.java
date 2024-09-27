package openbackup.system.base.common.model;

import lombok.Data;

/**
 * CreateUserLogParam
 *
 * @author y30046482
 * @since 2023-09-08
 */
@Data
public class CreateUserLogParam {
    private String platform;
    private String userName;
    private String ip;
}
