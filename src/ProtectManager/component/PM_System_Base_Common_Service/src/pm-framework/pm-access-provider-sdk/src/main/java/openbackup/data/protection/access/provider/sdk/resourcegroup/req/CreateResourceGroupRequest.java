package openbackup.data.protection.access.provider.sdk.resourcegroup.req;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import java.util.List;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

/**
 * 创建资源组请求体
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-01-23
 */

@Getter
@Setter
public class CreateResourceGroupRequest {
    @Length(min = 1, max = 64)
    @NotNull
    @Pattern(regexp = RegexpConstants.NAME_STR_NOT_START_WITH_NUM, message = "resource group name is invalid")
    private String name;

    @Length(max = 1024)
    @NotNull
    private String path;

    @Length(max = 64)
    @NotNull
    private String sourceType;

    @Length(max = 64)
    @NotNull
    private String sourceSubType;

    @NotNull
    private List<String> resourceIds;

    @Length(min = 1, max = 64)
    @NotNull
    private String scopeResourceId;
}