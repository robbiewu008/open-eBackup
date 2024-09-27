package openbackup.data.access.client.sdk.api.framework.dme.model;

import openbackup.data.access.client.sdk.api.framework.dme.DmeMountQos;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.system.base.sdk.resource.model.CifsProtocol;
import openbackup.system.base.sdk.resource.model.NfsProtocol;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 功能描述
 *
 * @author y30044273
 * @since 2023-11-17
 * @version [OceanProtect DataBackup 1.6.0]
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@Builder
public class AgentLessActionRequest {
    private StorageRepository repository;

    private NfsProtocol nfsProtocol;

    private CifsProtocol cifsProtocol;

    private String filesystemName;

    private DmeMountQos qos;

    private List<VpcInfo> vpcInfos;
}
