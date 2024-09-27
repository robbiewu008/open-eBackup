package openbackup.data.protection.access.provider.sdk.agent;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import lombok.Builder;
import lombok.Getter;
import lombok.Setter;

import java.util.Map;

/**
 * selector选择实体类
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/7/17
 */
@Getter
@Setter
@Builder
public class AgentSelectParam {
    private ProtectedResource resource;
    private String jobType;
    private Map<String, String> parameters;
}
