/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 */

package openbackup.system.base.common.utils.unit;

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.text.NumberFormat;
import java.util.logging.Logger;

/**
 * 数学工具类
 *
 * @author z90001721
 * @version V100R001C00
 * @since 2019-10-25
 */
public final class MathUtil {
    private static final Logger LOGGER =
            Logger.getLogger(MathUtil.class.getName());

    private static final int PRECISION_TWO = 2;

    private static final int PRECISION_ONE = 1;

    /**
     * 私有构造函数
     */
    private MathUtil() {
    }

    /**
     * 根据double提供的精度四舍五入
     *
     * @param number 需要转的数字
     * @param scale  精度
     * @return double 转换后的double
     */
    public static double downScaleToDouble(double number, int... scale) {
        int precision = PRECISION_TWO;
        if (scale.length >= PRECISION_ONE) {
            precision = scale[0];
        }
        BigDecimal bigDec = new BigDecimal(number);
        BigDecimal one = new BigDecimal(PRECISION_ONE);
        BigDecimal newBig = bigDec.divide(one, precision, BigDecimal.ROUND_DOWN);
        return newBig.doubleValue();
    }

    /**
     * 向下转换
     *
     * @param number 数值
     * @param scale  精度
     * @return String 小数位数
     */
    public static String downScaleToString(double number, int... scale) {
        int precision = PRECISION_TWO;
        if (scale.length >= PRECISION_ONE) {
            precision = scale[0];
        }
        BigDecimal bigDec = new BigDecimal(number);
        BigDecimal one = new BigDecimal(PRECISION_ONE);
        BigDecimal newBig = bigDec.divide(one, precision, BigDecimal.ROUND_DOWN);
        return newBig.toString();
    }

    /**
     * 计算百分比,不带百分号
     *
     * @param p1    数值1
     * @param p2    数值2
     * @param scale 小数位数
     * @return String [返回类型说明]
     */
    public static String computePercentNoSign(BigDecimal p1, BigDecimal p2, int scale) {
        String perCent = computePercent(p1, p2, scale);
        return perCent.replace("%", "");
    }

    /**
     * 计算百分比
     *
     * @param p1    数值1
     * @param p2    数值2
     * @param scale 小数位数
     * @return String [返回类型说明]
     */
    public static String computePercent(BigDecimal p1, BigDecimal p2, int scale) {
        if (p2.compareTo(BigDecimal.ZERO) <= 0) {
            return "0";
        }
        NumberFormat nf = NumberFormat.getPercentInstance();
        nf.setMinimumFractionDigits(scale);
        nf.setRoundingMode(RoundingMode.DOWN);
        nf.setMaximumFractionDigits(scale);
        BigDecimal quotient = p1.divide(p2, scale, RoundingMode.DOWN);
        return nf.format(quotient.doubleValue());
    }

    /**
     * 四舍五入格式化处理数字
     * 非点分形式，默认2位精度
     *
     * @param val       需要转换的值
     * @param precision 第一个为最大小数点位数，第二个参数为最小小数点位数
     * @return String
     */
    public static String parseNumber(Object val, int... precision) {
        return parseNumber(val, false, precision);
    }

    /**
     * 四舍五入格式化处理数字
     * 可选点分形式，默认2位精度
     *
     * @param val            需要转换的值
     * @param isGroupingUsed 是否点分显示
     * @param precision      第一个为最大小数点位数，第二个参数为最小小数点位数
     * @return String
     */
    public static String parseNumber(Object val, boolean isGroupingUsed, int... precision) {
        int max = PRECISION_TWO;
        int min = PRECISION_TWO;
        if (precision.length == PRECISION_ONE) {
            max = precision[0];
        } else if (precision.length == PRECISION_TWO) {
            max = precision[0];
            min = precision[1];
        } else {
            LOGGER.info("precision.length error");
        }
        NumberFormat nf = NumberFormat.getNumberInstance();
        // 添加舍入模式
        nf.setRoundingMode(RoundingMode.DOWN);
        nf.setMaximumFractionDigits(max);
        nf.setMinimumFractionDigits(Math.min(min, max));
        nf.setGroupingUsed(isGroupingUsed);
        return nf.format(val);
    }

    /**
     * 计算百分比,不带百分号
     *
     * @param number1   数值1
     * @param number2    数值2
     * @param scale 小数位数
     * @return String [返回类型说明]
     */
    public static String computePercentNoSignWithTwoDecimal(double number1, double number2, int scale) {
        String perCent = computePercentWithTwoDecimal(number1, number2, scale);
        return perCent.replace("%", "");
    }

    /**
     * 计算百分比
     *
     * @param number1    数值1
     * @param number2    数值2
     * @param scale 小数位数
     * @return String [返回类型说明]
     */
    public static String computePercentWithTwoDecimal(double number1, double number2, int scale) {
        if (number2 <= 0) {
            return "0";
        }

        NumberFormat nf = NumberFormat.getPercentInstance();
        nf.setMinimumFractionDigits(scale);
        nf.setRoundingMode(RoundingMode.DOWN);
        nf.setMaximumFractionDigits(scale);

        BigDecimal dividends = BigDecimal.valueOf(number1);
        BigDecimal divisor = BigDecimal.valueOf(number2);
        BigDecimal quotient = dividends.divide(divisor, scale + PRECISION_TWO, BigDecimal.ROUND_DOWN);
        return nf.format(quotient);
    }
}
