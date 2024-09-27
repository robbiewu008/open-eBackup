package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.security.Base64;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.charset.StandardCharsets;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Collection;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * 提供公共的校驗方法，如非空，正則表達式校驗等等
 *
 * @author w00448845
 * @version [CDM Integrated machine]
 * @since 2019-11-09
 */
public final class CommUtils {
    private static final Logger logger = LoggerFactory.getLogger(CommonUtil.class);

    private static final int APM_DESC_PARAM_MAX_SIZE = 255;

    private static final int APM_PORT_PARAM_MAX_SZIE = 65535;

    /**
     * 中划线
     */
    private static final String MIDLINE = "-";

    private CommUtils() {
    }

    /**
     * 判断字符串是否为空
     *
     * @param str 待校验字符串
     * @return boolean 结果
     */
    public static boolean isNullStr(String str) {
        return str == null || str.length() < 1;
    }

    /**
     * 通过正则表达式校验字符串
     *
     * @param str 字符串
     * @param regex 正则表达式
     * @return boolean 结果
     */
    public static boolean checkStr(String str, String regex) {
        if (isNullStr(str)) {
            return false;
        }

        if (regex == null) {
            return true;
        }

        Pattern pattern = Pattern.compile(regex);
        Matcher matcher = pattern.matcher(java.text.Normalizer.normalize(str, java.text.Normalizer.Form.NFKC));
        return matcher.matches();
    }

    /**
     * 判断集合是否为空
     *
     * @param collection 集合
     */
    public static void isNullCollection(Collection<?> collection) {
        if (collection == null || collection.size() < 1) {
            throw new EmeiStorDefaultExceptionHandler("system error");
        }
    }

    /**
     * 格式化日期 日期格式为UTC格式 yyyy-MM-dd HH:mm:ss UTC+/
     *
     * @param date 格式化日期
     * @return 日期字符串
     */
    public static String formatDate(Date date) {
        if (date == null) {
            return null;
        }
        // 格式化时间
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        // 组装时间
        return dateFormat.format(date) + (" " + TimeUtil.getDefaultTimeZone(date));
    }

    /**
     * 将毫秒时间转换成标准的时间,日期字符串 格式为yyyy-MM-dd HH:mm:ss
     *
     * @param date 毫秒
     * @return String
     */
    public static String formatDate(String date) {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        if (VerifyUtil.isEmpty(date)) {
            return null;
        }
        try {
            // 适配升级上来存在date结构为yyyy-MM-dd HH:mm:ss的逻辑
            Date date1 = format.parse(date);
            return format.format(date1);
        } catch (ParseException e) {
            return format.format(new Date(NumberUtil.convertToLong(date)));
        }
    }

    /**
     * 解析日期
     *
     * @param str 日期字符串 格式为yyyy-MM-dd HH:mm:ss
     * @return 日期
     */
    public static Date parseDate(String str) {
        if (isNullStr(str)) {
            return null;
        }
        Date result;
        try {
            SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
            result = format.parse(str);
        } catch (ParseException e) {
            logger.error("ParseException");
            return null;
        }
        return result;
    }

    /**
     * 匹配字符串是否包含IP
     *
     * @param matcherStr 匹配串
     * @return 是否包含ip。
     */
    public static boolean containIp(String matcherStr) {
        if (VerifyUtil.isEmpty(matcherStr)) {
            return false;
        }

        String matchIp = "\\d*((2[5][0-5]|2[0-4]\\d|1\\d{2}|\\d{1,2})\\." + "(25[0-5]|2[0-4]\\d|1\\d{2}|\\d{1,2})\\."
                + "(25[0-5]|2[0-4]\\d|1\\d{2}|\\d{1,2})\\." + "(25[0-5]|2[0-4]\\d|1\\d{2}|\\d{1,2}))";
        return Pattern.compile(matchIp)
                .matcher(java.text.Normalizer.normalize(matcherStr, java.text.Normalizer.Form.NFKC)).find();
    }

    /**
     * 字符串转换
     *
     * @param intStr 整形字符串
     * @return 转换后的值
     */
    public static int valueOfInt(String intStr) {
        if (VerifyUtil.isEmpty(intStr)) {
            throw new EmeiStorDefaultExceptionHandler(intStr);
        }

        try {
            return Integer.parseInt(intStr);
        } catch (Exception e) {
            throw new EmeiStorDefaultExceptionHandler(e.toString());
        }
    }

    /**
     * 整型值处理
     *
     * @param val val
     * @param minVal 最大值
     * @param maxVal 最小值
     * @return int val
     */
    public static int valueOfInt(int val, int minVal, int maxVal) {
        if (val < minVal || val > maxVal) {
            logger.error("Val: {}, minVal: {}, maxVal: {}.", val, minVal, maxVal);
            throw new EmeiStorDefaultExceptionHandler("minVal: " + minVal);
        }

        return val;
    }

    /**
     * json格式的字符串转化为数组
     *
     * @param jsonStr json格式字符串
     * @param fieldName 文件名称
     * @return String[] 提取出的字符串数组
     */
    public static String[] parseJsonStr2Array(String jsonStr, String fieldName) {
        JSONArray jsonArray;
        String[] restIdArray;
        try {
            jsonArray = JSONArray.fromObject(jsonStr);
            int size = jsonArray.size();
            restIdArray = new String[size];
            for (int i = 0; i < size; i++) {
                JSONObject object = JSONObject.fromObject(jsonArray.get(i));
                restIdArray[i] = object.getString(fieldName);
            }
        } catch (Exception e) {
            throw new EmeiStorDefaultExceptionHandler(e.toString());
        }
        return restIdArray;
    }

    /**
     * 数组参数检查
     *
     * @param params 要判断的数组
     * @return boolean
     */
    public static boolean checkArrayEmpty(Object[] params) {
        if (VerifyUtil.isEmpty(params)) {
            throw new EmeiStorDefaultExceptionHandler(Arrays.toString(params));
        }

        for (Object param : params) {
            if (VerifyUtil.isEmpty(param)) {
                throw new EmeiStorDefaultExceptionHandler(param.toString());
            }
            if (param instanceof String) {
                String pString = (String) param;
                if (VerifyUtil.isEmpty(pString)) {
                    throw new EmeiStorDefaultExceptionHandler(pString);
                }
            }
        }
        return true;
    }

    /**
     * 检查参数列表中的每个参数是否为空,只要其中的一个参数为空则抛出异常
     *
     * @param verifyList void
     */
    public static void verifyParameterList(List<String> verifyList) {
        if (VerifyUtil.isEmpty(verifyList)) {
            return;
        }

        for (String param : verifyList) {
            if (VerifyUtil.isEmpty(param)) {
                throw new EmeiStorDefaultExceptionHandler(param);
            }
        }
    }

    /**
     * 检查参数列表中的每个参数是否为空
     *
     * @param verifyMap 需要验证的map参数
     */
    @SuppressWarnings("unchecked")
    public static void verifyParameterMap(Map verifyMap) {
        if (VerifyUtil.isEmpty(verifyMap)) {
            return;
        }

        Iterator<Map.Entry> entries = verifyMap.entrySet().iterator();
        while (entries.hasNext()) {
            Map.Entry entry = entries.next();
            if (VerifyUtil.isEmpty(entry.getKey())) {
                logger.error("Map param key is null.");
                throw new EmeiStorDefaultExceptionHandler("Map param key is null");
            }
        }
    }

    /**
     * 验证set param
     *
     * @param sets 要判断的set
     */
    public static void verifyParameterSet(Set<Object> sets) {
        if (VerifyUtil.isEmpty(sets)) {
            throw new EmeiStorDefaultExceptionHandler("Map param key is null.");
        }

        for (Object set : sets) {
            if (VerifyUtil.isEmpty(set)) {
                throw new EmeiStorDefaultExceptionHandler("Map value can't be null.");
            }
        }
    }

    /**
     * JSON转换器
     *
     * @param object void
     * @return json obj
     */
    public static String responseJsonResult(Object object) {
        return JSONObject.fromObject(object).toString();
    }

    /**
     * JSON转换器
     *
     * @param object object
     * @param excludes void
     * @return json obj
     */
    public static String responseJsonResult(Object object, String[] excludes) {
        return JSONObject.fromObject(object, excludes).toString();
    }

    /**
     * JSON转换器
     *
     * @param objects void
     * @return json obj
     */
    public static String responseJsonArrayResult(List<?> objects) {
        return JSONArray.fromObject(objects).toString();
    }

    /**
     * JSON转换器
     *
     * @param objects obj
     * @param excludes void
     * @return json obj
     */
    public static String responseJsonArrayResult(Collection<?> objects, String[] excludes) {
        JSONArray array = new JSONArray();
        for (Object obj : objects) {
            JSONObject json = JSONObject.fromObject(obj, excludes);
            array.add(json);
        }
        return array.toString();
    }

    /**
     * 参数检查
     *
     * @param param 参数值
     * @param match 匹配模式key
     * @return 参数是否是合法
     */
    public static boolean checkParam(String param, String match) {
        // 参数检查
        if (VerifyUtil.isEmpty(param) || VerifyUtil.isEmpty(match)) {
            StringBuilder sb = new StringBuilder();
            sb.append("check param is error.");
            sb.append(" regEx :");
            sb.append(match);
            sb.append(" is error.");
            logger.error(sb.toString());
            throw new EmeiStorDefaultExceptionHandler(sb.toString());
        }

        // 用正则表达式匹配
        Pattern pat = Pattern.compile(match);
        Matcher mat = pat.matcher(java.text.Normalizer.normalize(param, java.text.Normalizer.Form.NFKC));

        return mat.matches() && mat.find(0) && mat.groupCount() >= 0;
    }

    /**
     * 参数检查
     *
     * @param param 参数值
     * @param maxSize 最大长度
     * @return 参数是否是合法
     */
    public static boolean checkParam(String param, int maxSize) {
        // 参数检查
        if (VerifyUtil.isEmpty(param)) {
            StringBuilder sb = new StringBuilder();
            sb.append("check param is error.");
            sb.append(" maxSize :");
            sb.append(maxSize);
            sb.append(" is error.");
            logger.error(sb.toString());
            throw new EmeiStorDefaultExceptionHandler(sb.toString());
        }

        // 判断长度是否满足
        return maxSize >= param.length();
    }

    /**
     * ip参数检查
     *
     * @param ip ip...
     * @return 参数是否是合法
     */
    public static boolean checkDrmIpParam(String ip) {
        if (VerifyUtil.isEmpty(ip) || ip.startsWith("127.") || ip.endsWith(".0") || ip.startsWith("0")) {
            return false;
        }

        String matchIp = "(2[2][0-3]|2[0-1]\\d|1\\d{2}|\\d{1,2})\\." + "(25[0-5]|2[0-4]\\d|1\\d{2}|\\d{1,2})\\."
                + "(25[0-5]|2[0-4]\\d|1\\d{2}|\\d{1,2})\\." + "(25[0-5]|2[0-4]\\d|1\\d{2}|\\d{1,2})";

        return checkParam(ip, matchIp);
    }

    /**
     * port参数检查
     *
     * @param portStr 对应APM界面上的名称
     * @return 参数是否是合法
     */
    public static boolean checkDrmPortParam(String portStr) {
        int port;
        try {
            port = Integer.parseInt(portStr);
        } catch (NumberFormatException e) {
            logger.error("Check port is error, param: {}.", portStr, ExceptionUtil.getErrorMessage(e));
            throw new EmeiStorDefaultExceptionHandler("Check port is error . param.");
        }

        return port > 0 && port <= APM_PORT_PARAM_MAX_SZIE;
    }

    /**
     * 描述检查
     *
     * @param description ...
     * @return 结果
     */
    public static boolean checkDrmDescParam(String description) {
        return VerifyUtil.isEmpty(description) || checkParam(description, APM_DESC_PARAM_MAX_SIZE);
    }

    /**
     * 格式化时间，如果时间格式为yyyy-MM-dd HH:mm:ss，直接返回，不需要再格式化
     *
     * @param data 时间
     * @return String yyyy-MM-dd HH:mm:ss
     */
    public static String formatData(String data) {
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

        try {
            sdf.parse(data);
            return data;
        } catch (ParseException ex) {
            logger.debug("time Parse failed: {}, error: {}", data, ExceptionUtil.getErrorMessage(ex).getMessage());
            // 如果日期格式不对，表示传来的为long型，需要格式化时间
            try {
                Long convertTime = NumberUtil.convertToLong(data);
                return formatDate(new Date(convertTime));
            } catch (NumberFormatException e) {
                return "--";
            }
        }
    }

    /**
     * 参数检查
     *
     * @param params 要检查的参数
     * @return 是否通过
     */
    public static boolean checkParamEmpty(Object[] params) {
        if (VerifyUtil.isEmpty(params)) {
            throw new EmeiStorDefaultExceptionHandler(Arrays.toString(params));
        }

        for (Object param : params) {
            if (VerifyUtil.isEmpty(param)) {
                throw new EmeiStorDefaultExceptionHandler(param.toString());
            }
        }

        return true;
    }

    /**
     * 用Base64解码entity
     *
     * @param id 对应entity
     * @return String
     */
    public static String decodeByBase64(String id) {
        byte[] idBytes = Base64.base64ToByteArray(id);
        return new String(idBytes, StandardCharsets.UTF_8);
    }

    /**
     * 用Base64解码entity
     *
     * @param id 对应entity
     * @return String
     */
    public static String encodeByBase64(String id) {
        return Base64.byteArrayToBase64(id.getBytes(StandardCharsets.UTF_8));
    }

    /**
     * 时间格式，将该时间格式的时间转换为时间戳
     *
     * @param data data
     * @return long
     */
    public static long dateToStamp(String data) {
        if (VerifyUtil.isEmpty(data)) {
            return 0L;
        }
        // 设置时间格式，将该时间格式的时间转换为时间戳
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        try {
            Date date = simpleDateFormat.parse(data);
            return date.getTime();
        } catch (ParseException ex) {
            logger.error("Time Parse failed: {}, error: {}.", data, ex.getMessage(), ExceptionUtil.getErrorMessage(ex));
            throw new EmeiStorDefaultExceptionHandler("Date Parse failed:, error.");
        }
    }

    /**
     * 转换为时间
     *
     * @param formatDate 时间字符串
     * @return 时间
     */
    public static Date parseDateFormatToDate(String formatDate) {
        SimpleDateFormat scheduleDateFormat;
        if (formatDate.contains("T")) {
            scheduleDateFormat = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss");
        } else {
            scheduleDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        }
        Date date;
        try {
            date = scheduleDateFormat.parse(formatDate);
        } catch (ParseException e) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                    "parse date failed, formatDate is" + formatDate, e.getCause());
        }
        return date;
    }

    /**
     * 拼接字符串
     *
     * @param strs 字符串数组
     * @return 拼接后的字符串
     */
    public static String combineString(String... strs) {
        return String.join(MIDLINE, strs);
    }
}
