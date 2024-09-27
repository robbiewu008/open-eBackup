package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.bean.DeviceUser;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * SyncComponentInfoRequest
 *
 * @author z00613137
 * @since 2023-05-25
 */
@Getter
@Setter
public class SyncComponentInfoRequest extends SyncComponentIpRequest {
    private List<ClusterComponentPwdInfo> componentPwdInfoList;

    private List<DeviceUser> deviceSecretList;
}