package openbackup.system.base.common.utils.unit;

import junit.framework.TestCase;
import openbackup.system.base.common.utils.unit.MathUtil;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述 MathUtil测试类
 *
 * @author s30031954
 * @since 2023-03-14
 */
public class MathUtilTest extends TestCase {
    /**
     * 用例场景：输入两位数字，及保留小数位数
     * 前置条件：无
     * 检查点：返回结果是否符合保留小数要求
     */
    @Test
    public void testComputePercent() {
        String result = MathUtil.computePercentNoSignWithTwoDecimal(381.488, 41456.64, 4);
        Assert.assertEquals(result, "0.9202");
    }
}