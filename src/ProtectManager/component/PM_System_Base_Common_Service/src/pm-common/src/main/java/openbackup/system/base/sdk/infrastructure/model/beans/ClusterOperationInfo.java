package openbackup.system.base.sdk.infrastructure.model.beans;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 集群操作信息
 *
 * @author x30046484
 * @since 2023-05-19
 */

@Getter
@Setter
public class ClusterOperationInfo {
    private String serviceName;
    private List<String> ipList;
}
