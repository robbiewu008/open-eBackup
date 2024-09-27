package openbackup.system.base.common.utils.unit;

/**
 * 容量单位：KB
 *
 * @author l90003110
 * @version V100R001C00
 * @since 2019-10-25
 */
public class KBCapUnitType extends AbstractCapUnitType {
    /**
     * 获得大的单位
     *
     * @return MBCapUnitType
     */
    @Override
    protected AbstractCapUnitType getBiggerCapUnitType() {
        // MB
        return new MBCapUnitType();
    }

    /**
     * 获得转换进制
     *
     * @return UNIT_KILO
     */
    @Override
    protected long getBiggerConversion() {
        return UNIT_KILO;
    }

    /**
     * 获得小一位的单位
     *
     * @return Byte
     */
    @Override
    protected AbstractCapUnitType getSmallerCapUnitType() {
        // Byte
        return new ByteCapUnitType();
    }

    /**
     * 向小转换的进制
     *
     * @return UNIT_KILO
     */
    @Override
    protected long getSmallerConversion() {
        return UNIT_KILO;
    }

    /**
     * 单位名称
     *
     * @return KB
     */
    @Override
    public String getUnitName() {
        return "KB";
    }
}
