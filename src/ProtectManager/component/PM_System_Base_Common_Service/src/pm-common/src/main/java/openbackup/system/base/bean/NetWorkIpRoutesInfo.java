package openbackup.system.base.bean;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * NetWorkIpRoutesInfo
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-20
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class NetWorkIpRoutesInfo {
    private String ip;

    private List<NetWorkRouteInfo> routes;
}
