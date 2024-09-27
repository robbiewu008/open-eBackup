package openbackup.data.protection.access.provider.sdk.resourcegroup.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 资源组保护结果
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-06-20
 */
@Getter
@Setter
public class ResourceGroupResultDto {
    /**
     * 资源名称
     */
    private String resourceName;

    /**
     * 是否成功
     */
    @JsonProperty("isSuccess")
    private boolean isSuccess;

    private Long errorCode;

    private String[] parameters;

    private String message;
}
