package openbackup.system.base.common.utils.unit;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;

import java.text.DecimalFormat;
import java.text.NumberFormat;

/**
 * 用于存放所有单位的枚举与转换逻辑
 *
 * @author x00102290
 * @version V100R001C00
 * @since 2019-10-25
 */
public final class UnitConvert {
    /**
     * 未知数据显示内容
     */
    private static final String UNKNOWN_DISPLAY_STRING = "--";

    /* * 六位精度 */
    private static final int SIX_PRECISION = 6;

    private static final String UNMATICHING_UNIT = "unmatiching unit";

    private static final String AUTO_CONVERT_UNIT_ERROR = "Auto convert unit error.";

    private UnitConvert() {
    }

    /**
     * 用于在同种单位之间进行换算
     *
     * @param value 要换算的原始值
     * @param srcUnit value指定的原单位
     * @param targetUnit 要换算成的目标单位
     * @return 换算结果
     */
    public static double convert(Double value, IUnitType srcUnit, IUnitType targetUnit) {
        if (value == null) {
            return 0D;
        }
        return convert(value, srcUnit, targetUnit, SIX_PRECISION);
    }

    /**
     * 用于在同种单位之间进行换算
     *
     * @param value 要换算的原始值
     * @param srcUnit value指定的原单位
     * @param targetUnit 要换算成的目标单位
     * @param scale 小数点后保留位数
     * @return 换算结果
     */
    public static double convert(double value, IUnitType srcUnit, IUnitType targetUnit, int scale) {
        if (scale < 1) {
            throw new IllegalArgumentException("Parameter scale must be not less than 1.");
        }

        if (!srcUnit.getClass().isInstance(targetUnit)) {
            throw new IllegalArgumentException(UNMATICHING_UNIT);
        }
        if (Double.valueOf(Double.MIN_VALUE).equals(value)) {
            return value;
        }
        double returnValue = value;
        if (srcUnit.getUnit() > targetUnit.getUnit()) {
            long tmp = srcUnit.getUnit() / targetUnit.getUnit();
            returnValue = (double) tmp * value;
        }
        if (srcUnit.getUnit() < targetUnit.getUnit()) {
            long tmp = targetUnit.getUnit() / srcUnit.getUnit();
            returnValue = value / (double) tmp;
        }
        return MathUtil.downScaleToDouble(returnValue, scale);
    }

    /**
     * 用于在同种单位之间进行换算
     *
     * @param value 要换算的原始值
     * @param srcUnit value指定的原单位
     * @param targetUnit 要换算成的目标单位
     * @return 换算结果
     */
    public static int convert(int value, IUnitType srcUnit, IUnitType targetUnit) {
        if (!srcUnit.getClass().isInstance(targetUnit)) {
            throw new IllegalArgumentException(UNMATICHING_UNIT);
        }

        if (value == Integer.MIN_VALUE) {
            return value;
        }

        long returnValue = value;
        if (srcUnit.getUnit() > targetUnit.getUnit()) {
            returnValue = (srcUnit.getUnit() / targetUnit.getUnit()) * value;
        }
        if (srcUnit.getUnit() < targetUnit.getUnit()) {
            returnValue = value / (targetUnit.getUnit() / srcUnit.getUnit());
        }
        return (int) returnValue;
    }

    /**
     * 用于在同种单位之间进行换算
     *
     * @param value 要换算的原始值
     * @param srcUnit value指定的原单位
     * @param targetUnit 要换算成的目标单位
     * @return 换算结果
     */
    public static long convert(long value, IUnitType srcUnit, IUnitType targetUnit) {
        if (!srcUnit.getClass().isInstance(targetUnit)) {
            throw new IllegalArgumentException(UNMATICHING_UNIT);
        }

        if (value == Long.MIN_VALUE) {
            return value;
        }

        long returnValue = value;
        if (srcUnit.getUnit() > targetUnit.getUnit()) {
            returnValue = (srcUnit.getUnit() / targetUnit.getUnit()) * value;
        }
        if (srcUnit.getUnit() < targetUnit.getUnit()) {
            returnValue = value / (targetUnit.getUnit() / srcUnit.getUnit());
        }
        return returnValue;
    }

    /**
     * 将容量自动转换至合适的单位
     * 满足“单位动态变化”及“2位小数精度”规范
     *
     * @param value 容量值 如：1024
     * @param srcUnit 容量原始单位 如：CapabilityUnitType.Byte
     * @param maxPrecision 最大支持精度
     * @param minPrecision 最小支持精度
     * @return String 自动转换后的容量值加单位字串 如：1.00 KB
     */
    public static String autoConvertToAdaptedValueAndUnit(Double value, CapabilityUnitType srcUnit, int maxPrecision,
        int minPrecision) {
        String valueAndUnitStr = UNKNOWN_DISPLAY_STRING;
        if (VerifyUtil.isEmpty(value)) {
            return valueAndUnitStr;
        }
        // 值是0时不要向下转（直接用默认单位）
        if (value <= 0) {
            NumberFormat nf = new DecimalFormat();
            nf.setMaximumFractionDigits(maxPrecision);
            nf.setMinimumFractionDigits(minPrecision);
            return nf.format(value) + " " + srcUnit.name();
        }

        try {
            AbstractCapUnitType unitType = CapUnitTypeFactory.getInstance().createCapUnitType(srcUnit);
            valueAndUnitStr = unitType.getAdaptedValueWithUnitByPrecision(value, maxPrecision, minPrecision);
        } catch (RuntimeException e) {
            throw new LegoCheckedException(AUTO_CONVERT_UNIT_ERROR, e);
        }
        return valueAndUnitStr;
    }
}
