package openbackup.data.protection.access.provider.sdk.index;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;

/**
 * 生成索引文件
 *
 * @author t00482481
 * @since 2020-08-26
 */
public interface IndexerProvider extends DataProtectionProvider<String> {
    /**
     * 生成索引文件
     *
     * @param requestId 请求id
     * @param copy 副本信息
     * @return 是否成功
     */
    boolean generateIndexFile(String requestId, CopyBo copy);
}
