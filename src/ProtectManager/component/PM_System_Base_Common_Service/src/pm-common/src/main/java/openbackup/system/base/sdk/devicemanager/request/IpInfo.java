package openbackup.system.base.sdk.devicemanager.request;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotNull;

/**
 * pacific 初始化网络时传的参数
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-03
 */
@Getter
@Setter
public class IpInfo {
    // ip地址/掩码
    @NotNull(message = "The ipAddress cannot be null")
    @Length(min = 1, max = 64)
    private String ipAddress;

    // 端口名称
    @NotNull(message = "The ifaceName cannot be null")
    @Length(min = 1, max = 64)
    private String ifaceName;
}
