package openbackup.tdsql.resources.access.dto.cluster;

import lombok.Data;

import java.util.List;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-05-23
 */
@Data
public class TdsqlCluster {
    private List<OssNode> ossNodes;

    private List<SchedulerNode> schedulerNodes;
}
