package openbackup.system.base.common.constants;

import lombok.Getter;
import lombok.Setter;

/**
 * 本地远程复制Pir
 *
 * @author g30003063
 * @since 2021-12-14
 */
@Getter
@Setter
public class LocalRemoteReplicationPair {
    /**
     * 远程复制PairID
     */
    private String id;

    /**
     * 本端是否是主端
     */
    private boolean isPrimary;

    /**
     * 复制模式
     * 1：同步
     * 2：异步
     */
    private int replicationModel;
}
