package openbackup.system.base.sdk.cluster.model.storage;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

/**
 * 存储单元查询请求参数体
 *
 * @author w00639094
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-20
 */
@AllArgsConstructor
@NoArgsConstructor
@Setter
@Getter
public class StorageUnitRequestParams {
    private List<String> storageUnitIds;
}
