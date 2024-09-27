package openbackup.system.base.common.utils.unit;

/**
 * 容量单位
 *
 * @author l90002863 李溜
 * @version V100R001C00
 * @since 2019-10-25
 */
public enum CapabilityUnitType implements IUnitType {
    /**
     * bit
     */
    BIT(1L),

    /**
     * byte
     */
    BYTE(8L),

    /**
     * KB
     */
    KB(1024L * 8L),

    /**
     * MB
     */
    MB((1024L * 1024L) * 8L),

    /**
     * GB
     */
    GB((1024L * 1024L * 1024L) * 8L),

    /**
     * TB
     */
    TB((1024L * 1024L * 1024L * 1024L) * 8L),

    /**
     * PB
     */
    PB((1024L * 1024L * 1024L * 1024L * 1024L) * 8L);

    private final long scale;

    private final boolean isGroupUsed = Boolean.TRUE;

    CapabilityUnitType(long scale) {
        this.scale = scale;
    }

    /**
     * getUnit
     *
     * @return long
     */
    public long getUnit() {
        return this.scale;
    }

    /**
     * isGroupingUsed
     *
     * @return boolean
     */
    @Override
    public boolean isGroupingUsed() {
        return isGroupUsed;
    }
}
