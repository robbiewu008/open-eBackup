package openbackup.system.base.common.utils;

import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;
import openbackup.system.base.common.model.storage.StorageSessionRequest;
import openbackup.system.base.common.utils.CommUtils;

import org.junit.Assert;
import org.junit.Test;

import java.util.*;

public class CommonUtilsTest {
    @Test
    public void testCheckStr() {
        Assert.assertFalse(CommUtils.checkStr("", null));
        Assert.assertTrue(CommUtils.checkStr("a", null));
        Assert.assertFalse(CommUtils.checkStr("a", "^[0-9]*"));
    }

    @Test(expected = EmeiStorDefaultExceptionHandler.class)
    public void testIsNullCollection() {
        CommUtils.isNullCollection(null);
    }

    @Test
    public void testFormatDate() {
        Assert.assertTrue(CommUtils.formatData("2010-12-02 20:02:01").contains("2010-12-02 20:02:01"));
        Assert.assertTrue(CommUtils.formatData("1234").contains("1970-01-01 08:00:01"));
        Date date = new Date(1234);
        Assert.assertTrue(CommUtils.formatDate(date).contains("1970-01-01 08:00:01"));
        Assert.assertNull(CommUtils.formatDate((Date) null));

        Assert.assertTrue(CommUtils.formatDate("2010-12-02 20:02:01").contains("2010-12-02 20:02:01"));
        Assert.assertTrue(CommUtils.formatDate("1234").contains("1970-01-01 08:00:01"));
        Assert.assertNull(CommUtils.formatDate((String) null));
        Assert.assertNull(CommUtils.parseDate(""));
        Date res = CommUtils.parseDate("1970-01-01 08:00:01");
        Assert.assertEquals(res.getTime(), 1000);
        Assert.assertNull(CommUtils.parseDate("1970:01:01"));
    }

    @Test
    public void testContainIp() {
        Assert.assertFalse(CommUtils.containIp(""));
        Assert.assertTrue(CommUtils.containIp("1.1.1.1"));
    }

    @Test
    public void testValueOfInt() {
        Assert.assertEquals(CommUtils.valueOfInt("123"), 123);
        Assert.assertEquals(CommUtils.valueOfInt(3, 1, 5), 3);
    }

    @Test(expected = EmeiStorDefaultExceptionHandler.class)
    public void testValueOfIntWhenNullRaiseException() {
        CommUtils.valueOfInt("");
    }

    @Test(expected = EmeiStorDefaultExceptionHandler.class)
    public void testValueOfIntWithLetterRaiseException() {
        CommUtils.valueOfInt("234dasd");
    }

    @Test(expected = EmeiStorDefaultExceptionHandler.class)
    public void testValueOfIntWhenMinValWrongRaiseException() {
        CommUtils.valueOfInt(3, 4, 1);
    }

    @Test
    public void testParseJson() {
        StorageSessionRequest request = new StorageSessionRequest();
        request.setUsername("admin");
        String[] expect = {"23", "25"};
        String jsonStr = "[{'day1':'work','day2':'23'},{'day1':123,'day2':'25'}]";
        String expectStr = "{\"username\":\"admin\",\"password\":\"\",\"scope\":0}";
        String expectExcludeStr = "{\"username\":\"admin\",\"password\":\"\"}";
        String[] res = CommUtils.parseJsonStr2Array(jsonStr, "day2");
        String[] excludes = {"scope"};
        List<String> list = Arrays.asList(expect);
        Collection<StorageSessionRequest> collection = new ArrayList<>();
        collection.add(request);
        Assert.assertEquals(CommUtils.combineString("ab", "cd"), "ab-cd");
        Assert.assertArrayEquals(expect, res);
        Assert.assertEquals(CommUtils.responseJsonResult(request), expectStr);
        Assert.assertEquals(CommUtils.responseJsonResult(request, excludes), expectExcludeStr);
        Assert.assertEquals(CommUtils.responseJsonArrayResult(list), "[\"23\",\"25\"]");
        Assert.assertEquals(CommUtils.responseJsonArrayResult(collection, excludes),
                "[{\"username\":\"admin\",\"password\":\"\"}]");
    }

    @Test
    public void testCheckParam() {
        String[] str = {"23", "123"};
        List<String> list = Arrays.asList(str);
        Map<String, String> map = new HashMap<>();
        map.put("id", "123");
        CommUtils.verifyParameterList(list);
        CommUtils.verifyParameterMap(map);
        CommUtils.verifyParameterList(null);
        CommUtils.verifyParameterMap(null);
        Assert.assertTrue(CommUtils.checkArrayEmpty(str));
        Assert.assertTrue(CommUtils.checkParam("abc", 5));
        Assert.assertFalse(CommUtils.checkDrmIpParam(""));
        Assert.assertTrue(CommUtils.checkDrmIpParam("1.1.1.1"));
        Assert.assertTrue(CommUtils.checkDrmPortParam("6432"));
        Assert.assertTrue(CommUtils.checkDrmDescParam(""));
        Assert.assertTrue(CommUtils.checkParamEmpty(new Object[]{"123"}));
    }

    @Test(expected = EmeiStorDefaultExceptionHandler.class)
    public void testArrayEmptyRaiseException() {
        CommUtils.checkArrayEmpty(null);
    }

    @Test(expected = EmeiStorDefaultExceptionHandler.class)
    public void testParameterListRaiseException() {
        String[] str = {"", "123"};
        List<String> list = Arrays.asList(str);
        CommUtils.verifyParameterList(list);
    }

    @Test(expected = EmeiStorDefaultExceptionHandler.class)
    public void testParameterMapRaiseException() {
        Map<String, String> map = new HashMap<>();
        map.put("", "123");
        CommUtils.verifyParameterMap(map);
    }

    @Test(expected = EmeiStorDefaultExceptionHandler.class)
    public void testParameterSetNullRaiseException() {
        CommUtils.verifyParameterSet(null);
    }

    @Test(expected = EmeiStorDefaultExceptionHandler.class)
    public void testParameterSetRaiseException() {
        Set<Object> set = new HashSet<>();
        set.add("");
        CommUtils.verifyParameterSet(set);
    }

    @Test(expected = EmeiStorDefaultExceptionHandler.class)
    public void testCheckParamWithTwoParamRaiseException() {
        CommUtils.checkParam("", "");
    }

    @Test(expected = EmeiStorDefaultExceptionHandler.class)
    public void testCheckParamWithEmptyParamRaiseException() {
        CommUtils.checkParam("", 2);
    }

    @Test(expected = EmeiStorDefaultExceptionHandler.class)
    public void testCheckParamEmptyRaiseException() {
        CommUtils.checkParamEmpty(new Object[]{""});
    }

    @Test(expected = EmeiStorDefaultExceptionHandler.class)
    public void testCheckParamContextEmptyRaiseException() {
        String[] str = {"", "123"};
        CommUtils.checkParamEmpty(str);
    }

    @Test
    public void testEncodeDecodeByBase64() {
        Assert.assertNotNull(CommUtils.decodeByBase64("1234"));
        Assert.assertNotNull(CommUtils.encodeByBase64("1234"));
    }

    @Test
    public void testDateToStamp() {
        Assert.assertEquals(CommUtils.dateToStamp(""), 0);
        Assert.assertEquals(CommUtils.dateToStamp("2010-12-02 20:02:01"), 1291291321000L);
    }
}
