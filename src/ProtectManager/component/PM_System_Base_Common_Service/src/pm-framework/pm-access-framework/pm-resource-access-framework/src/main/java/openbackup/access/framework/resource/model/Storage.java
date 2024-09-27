package openbackup.access.framework.resource.model;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotBlank;

/**
 * 功能描述: 生产存储
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-22
 */
@Getter
@Setter
public class Storage {
    @NotBlank
    @Length(max = 128)
    private String username;

    @NotBlank
    @Length(max = 128)
    private String password;

    @NotBlank
    @Length(max = 32)
    private String ip;

    @Length(max = 300)
    private String ipList;

    private int port;

    @Length(max = 1)
    private String enableCert;

    // HCS存储的证书内容
    @Length(max = 50000)
    private String certification;

    @Length(max = 128)
    private String revocationList;

    @Length(max = 128)
    private String certName;

    @Length(max = 128)
    private String certSize;

    @Length(max = 128)
    private String crlName;

    @Length(max = 128)
    private String crlSize;

    @Length(max = 128)
    private String sn;

    @Length(max = 128)
    private String storageType;
}