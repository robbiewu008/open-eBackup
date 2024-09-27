package openbackup.data.protection.access.provider.sdk.replication;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyReplicationImport;

/**
 * Replication Provider
 *
 * @author l00272247
 * @since 2020-11-18
 */
public interface ReplicationProvider extends DataProtectionProvider<String> {
    /**
     * replicate backup object
     *
     * @param context context
     */
    void replicate(IReplicateContext context);

    /**
     * build copy info
     *
     * @param copy copy
     * @param importParam import param
     */
    void buildCopyProperties(CopyInfoBo copy, CopyReplicationImport importParam);

    /**
     * check copy whether exist
     *
     * @param chainId chainId
     * @param timestamp timestamp
     * @return 待入库副本是否已存在
     */
    boolean checkCopyWhetherExist(String chainId, long timestamp);
}
