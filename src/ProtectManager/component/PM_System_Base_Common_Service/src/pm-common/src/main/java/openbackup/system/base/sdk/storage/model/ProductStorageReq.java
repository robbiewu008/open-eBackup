package openbackup.system.base.sdk.storage.model;

import openbackup.system.base.common.constants.IsmConstant;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Data;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Max;
import javax.validation.constraints.Pattern;

/**
 * 存储信息
 *
 * @author p00511147
 * @since 2020-11-10
 */
@Data
public class ProductStorageReq {
    @Length(max = 32)
    private String userName;

    @Length(max = 32)
    private String password;

    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "ip is invalid, not ipv4 or ipv6.")
    private String ip;

    @Max(IsmConstant.PORT_MAX)
    private int port;
}
