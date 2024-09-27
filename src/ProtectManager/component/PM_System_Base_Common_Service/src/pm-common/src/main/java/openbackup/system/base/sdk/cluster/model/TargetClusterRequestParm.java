package openbackup.system.base.sdk.cluster.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 外部集群查询参数对象
 *
 * @author nwx1077006
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-15
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class TargetClusterRequestParm {
    private List<Integer> clusterIdList;

    private List<String> esnList;

    private Integer status;

    private Integer generatedType;

    private String remoteEsn;

    private List<Integer> roleList;

    public TargetClusterRequestParm(List<Integer> clusterIdList) {
        this.clusterIdList = clusterIdList;
    }
}
