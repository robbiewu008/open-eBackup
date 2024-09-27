package openbackup.data.protection.access.provider.sdk.resource;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.Map;

/**
 * 资源查询context
 *
 * @author h30027154
 * @since 2022-07-08
 */
@Data
public class ResourceQueryParams {
    private boolean shouldDecrypt = false;

    private boolean shouldQueryDependency = false;

    private boolean isDesesitization = false;

    private boolean shouldLoadEnvironment = true;

    // 查询条件是否忽略资源的拥有者
    private boolean shouldIgnoreOwner = false;

    @JsonProperty("pageNo")
    private int page = 0;

    @JsonProperty("pageSize")
    private int size = 10;

    private Map<String, Object> conditions;

    private String[] orders = new String[0];
}
