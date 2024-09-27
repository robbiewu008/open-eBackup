package openbackup.system.base.common.utils.unit;

/**
 * 容量单位：Byte
 *
 * @author l90003110
 * @version V100R001C00
 * @since 2019-10-25
 */
public class ByteCapUnitType extends AbstractCapUnitType {
    /**
     * 获取KBCapUnitType值
     *
     * @return KBCapUnitType
     */
    @Override
    protected AbstractCapUnitType getBiggerCapUnitType() {
        // KB
        return new KBCapUnitType();
    }

    /**
     * 获取BiggerConversion值
     *
     * @return UNIT_KILO
     */
    @Override
    protected long getBiggerConversion() {
        return UNIT_KILO;
    }

    /**
     * 获取SmallerCapUnitType。默认为null
     *
     * @return AbstractCapUnitType
     */
    @Override
    protected AbstractCapUnitType getSmallerCapUnitType() {
        // Bit
        // 最小单位裁定为Byte
        return null;
    }

    /**
     * 获取小的转换
     *
     * @return Long.MIN_VALUE
     */
    @Override
    protected long getSmallerConversion() {
        return Long.MIN_VALUE;
    }

    /**
     * 获取单位名称
     *
     * @return Byte
     */
    @Override
    public String getUnitName() {
        return "Byte";
    }
}
