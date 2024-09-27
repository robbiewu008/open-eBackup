package openbackup.system.base.sdk.cluster.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 查询历史容量信息
 *
 * @author z00613137
 * @since 2023-05-11
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class ClusterStorageTendencyInfo {
    private Integer clusterId;

    private String esn;

    private int peakPoint;

    private List<ClusterStorageTendencyDayInfo> existingDatas;

    private List<ClusterStorageTendencyDayInfo> forecastDatas;
}