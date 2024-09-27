package openbackup.system.base.common.utils.unit;

/**
 * 容量单位：MB
 *
 * @author l90003110
 * @version V100R001C00
 * @since 2019-10-25
 */
public class MBCapUnitType extends AbstractCapUnitType {
    /**
     * 获得大的单位
     *
     * @return GBCapUnitType
     */
    @Override
    protected AbstractCapUnitType getBiggerCapUnitType() {
        // GB
        return new GBCapUnitType();
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
     * @return KBCapUnitType
     */
    @Override
    protected AbstractCapUnitType getSmallerCapUnitType() {
        // KB
        return new KBCapUnitType();
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
     * @return MB
     */
    @Override
    public String getUnitName() {
        return "MB";
    }
}
