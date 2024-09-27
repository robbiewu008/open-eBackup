package openbackup.system.base.common.utils;

import openbackup.system.base.common.utils.DateFormatUtil;

import org.junit.Assert;
import org.junit.Test;

import java.text.DateFormat;
import java.text.ParseException;
import java.util.Date;
import java.util.Locale;

public class DateFormatUtilTest {
    @Test
    public void testFormat() {
        DateFormat dateFormat = DateFormat.getDateInstance(DateFormat.DEFAULT, Locale.ENGLISH);
        Date date = new Date(1234);
        Assert.assertEquals(DateFormatUtil.format(dateFormat, date), "Jan 1, 1970");
        Assert.assertEquals(DateFormatUtil.format(dateFormat, (Object) date), "Jan 1, 1970");
    }

    @Test(expected = ParseException.class)
    public void testFormatRaiseException() throws Exception {
        DateFormat dateFormat = DateFormat.getDateInstance(DateFormat.DEFAULT, Locale.ENGLISH);
        DateFormatUtil.parse(dateFormat, "2010-12-02 20:02:01");
    }
}
