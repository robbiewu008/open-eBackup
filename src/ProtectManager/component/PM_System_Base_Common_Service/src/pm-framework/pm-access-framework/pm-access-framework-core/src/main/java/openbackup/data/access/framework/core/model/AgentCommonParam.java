package openbackup.data.access.framework.core.model;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 调用agent接口参数
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/2/28
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class AgentCommonParam {
    private String appType;
    private String endpoint;
    private Integer port;
    private boolean isRetry;
}
