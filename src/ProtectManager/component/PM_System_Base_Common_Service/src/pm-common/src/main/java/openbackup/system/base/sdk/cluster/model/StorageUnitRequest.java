package openbackup.system.base.sdk.cluster.model;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

/**
 * 存储单元接口请求类
 *
 * @author w00639094
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-06
 */
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
public class StorageUnitRequest {
    @Length(max = 256)
    private String name;

    @Length(max = 1024)
    private String deviceId;

    @Length(max = 256)
    private String poolId;

    @Length(max = 64)
    private String deviceType;
}
