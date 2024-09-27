package openbackup.system.base.sdk.cluster.request;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotNull;

/**
 * 更新存储池容量阈值请求体
 *
 * @author w00639094
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-12
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class StoragePoolThresholdRequest {
    @NotNull
    @Length(max = 64)
    private String deviceId;

    @NotNull
    @Length(max = 64)
    private String poolId;

    @NotNull
    @Length(max = 64)
    private String threshold;
}
