package openbackup.system.base.bean;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 对Secret进行操作
 *
 * @author l00853347
 * @since 2023-12-21
 * @version [OceanProtect DataBackup 1.6.0]
 */
@Setter
@Getter
public class SecretInfo {
    /**
     * 设备ID
     */
    private String id;

    /**
     * 设备下面的用户
     */
    private List<DeviceUser> deviceUser;
}
