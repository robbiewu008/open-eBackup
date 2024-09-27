package openbackup.system.base.sdk.devicemanager.request;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

import javax.validation.Valid;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

/**
 * 更新pacific 集群业务网络配置请求体
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-03-23
 */
@Getter
@Setter
public class UpdateBusinessNetworkRequest {
    @Valid
    @NotNull
    @Size(min = 1, message = "The backup network pacific network info is configured with at least one property")
    private List<NodeNetworkInfoRequest> backupNetWorkInfoList;

    @Valid
    private List<NodeNetworkInfoRequest> archiveNetWorkInfoList;
}
