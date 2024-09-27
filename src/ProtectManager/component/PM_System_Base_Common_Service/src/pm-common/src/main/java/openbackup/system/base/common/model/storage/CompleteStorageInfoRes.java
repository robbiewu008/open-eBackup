package openbackup.system.base.common.model.storage;

import lombok.Data;

import java.math.BigDecimal;

/**
 * 存储库完整信息回复
 *
 * @author w00504341
 * @since 2020-12-18
 */
@Data
public class CompleteStorageInfoRes {
    private String storageName;

    private String endpoint;

    private String certName;

    private String certId;

    private int type;

    private int cloudType;

    private int port;

    private Integer connectType;

    private String ak;

    private String bucketName;

    private String indexBucketName;

    private boolean proxyEnable;

    private String proxyHostName;

    private String proxyUserName;

    private boolean useHttps;

    private boolean alarmEnable;

    private long alarmThreshold;

    private String noSensitiveSk;

    /**
     * 容量告警单位
     * 取值定义请见 @see com.huawei.oceanprotect.repository.common.enums.RepositoryThresholdAlarmUnit
     */
    private String alarmLimitValueUnit;

    private int status;

    private BigDecimal totalSize;

    private BigDecimal usedSize;

    private BigDecimal freeSize;
}
