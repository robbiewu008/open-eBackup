package openbackup.system.base.common.model.hcs;

import lombok.Getter;
import lombok.Setter;

/**
 * 功能描述 磁盘信息
 *
 * @author z30027603
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/13 17:54
 */
@Getter
@Setter
public class DiskInfo {
    private String uuid;

    private String id;

    private String name;

    private String mode;

    private String attr;

    private String size;

    private String lunWWN;

    private String shareable;

    private String architecture;

    private String serverId;

    private String sn;

    private String storageManagerIp;

    private String storageType;

    /**
     * 是否是加密盘
     */
    private String systemEncrypted;

    private String systemCmkId;

    private String cipher;
}
