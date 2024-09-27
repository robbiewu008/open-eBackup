package openbackup.system.base.bean;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * NetWorkLogicIp
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-20
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class NetWorkLogicIp {
    private String ip;

    private String mask;
}
