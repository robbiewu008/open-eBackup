package openbackup.system.base.sdk.infrastructure.model.beans;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 证书替换所需的证书类型
 *
 * @author yWX1126359
 * @since 2022/2/19
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class CertificateTypeInfo {
    /**
     * 直接重启，不拷贝证书
     */
    public static final String OTHERS = "others";

    // 内部通信服务或者redis证书类型
    @JsonProperty(value = "cert_type")
    private String certType;
}
