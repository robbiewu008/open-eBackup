package openbackup.system.base.common.utils.unit;

/**
 * 容量单位：GB
 *
 * @author l90003110
 * @version V100R001C00
 * @since 2019-10-25
 */
public class GBCapUnitType extends AbstractCapUnitType {
    /**
     * 获得大的单位
     *
     * @return TBCapUnitType
     */
    @Override
    protected AbstractCapUnitType getBiggerCapUnitType() {
        // PB
        return new TBCapUnitType();
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
     * @return MB
     */
    @Override
    protected AbstractCapUnitType getSmallerCapUnitType() {
        // MB
        return new MBCapUnitType();
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
     * @return GB
     */
    @Override
    public String getUnitName() {
        return "GB";
    }
}
