package openbackup.system.base.config;

import openbackup.system.base.common.enums.ServiceType;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import javax.validation.constraints.Pattern;

/**
 * 标准备份服务 参数
 *
 * @author swx1010572
 * @since 2021-09-26
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class StandardBackupServiceConfig {
    /**
     * 浮动IP
     */
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "please input correct floating ip address")
    private String serviceIp;

    /**
     * 服务类型
     */
    private ServiceType serviceType;
}
