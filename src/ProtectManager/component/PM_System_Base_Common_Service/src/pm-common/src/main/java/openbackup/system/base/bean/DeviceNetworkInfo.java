package openbackup.system.base.bean;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * DeviceNetworkInfo
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-29
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class DeviceNetworkInfo {
    List<NetWorkConfigInfo> backupConfig;
    List<NetWorkConfigInfo> archiveConfig;
    List<NetWorkConfigInfo> replicationConfig;
}
