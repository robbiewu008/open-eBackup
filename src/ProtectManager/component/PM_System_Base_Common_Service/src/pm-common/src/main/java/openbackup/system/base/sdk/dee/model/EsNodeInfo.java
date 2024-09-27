package openbackup.system.base.sdk.dee.model;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Data;

import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 功能描述
 *
 * @author c30047317
 * @since 2023-08-12
 */
@Data
public class EsNodeInfo {
    // 节点IP
    @Size(min = 1, max = 16)
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "ip is invalid")
    private String ip;

    // 节点对应机器的ESN
    @Size(min = 1, max = 64)
    private String esn;
}