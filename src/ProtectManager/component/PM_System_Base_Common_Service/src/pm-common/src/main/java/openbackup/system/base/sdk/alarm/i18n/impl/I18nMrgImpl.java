/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.sdk.alarm.i18n.impl;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONException;
import com.alibaba.fastjson.JSONObject;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.MessageFormatUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.alarm.i18n.I18nMrg;
import openbackup.system.base.sdk.alarm.i18n.I18nMrgUtil;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.AdapterUtils;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.io.IOUtils;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.core.annotation.Order;
import org.springframework.lang.NonNull;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.annotation.PostConstruct;

/**
 * 后台资源国际化对外的接口
 *
 */
@Slf4j
@Service
@Order(197)
public class I18nMrgImpl implements I18nMrg, InitializingBean {
    private static final Map<Locale, Map<String, String>> RES_MAP = new HashMap<>();

    private static final Map<String, Map<Locale, Map<String, String>>> DEPLOY_I18N_MAP = new HashMap<>();

    private static final Map<String, List<String>> PARAM_EXPLAIN_MAP = new HashMap<>();

    private static final String EMPTY_STRING = "";

    private static final String DEFAULT_STRING = "--";

    private static final String ENGLISH_LANG = "en";

    // 大括号匹配符
    private static Pattern bracePattern = Pattern.compile("\\{.*?\\}");

    // 中括号匹配符
    private static Pattern bracketPattern = Pattern.compile("(\\[.*?\\])");

    // 匹配格式： []{}, []中不含有[
    private static Pattern braceAndBracketPatter = Pattern.compile("^\\[[^\\[]*?\\]\\s*?(\\{.*?\\})");

    /**
     * 不处理的底座告警，原来定义在read_dorado_alarms_from_local.py中
     */
    private static final Set<String> NOT_HANDLE_ALARMS = new HashSet<>(Arrays.asList("0xF0C90001", "0xF0C90002",
            "0x2064032B0001", "0x2064032B0007", "0x2064032B0008", "0x2064032B0009", "0x2064032B000A", "0x2064032B000B",
            "0x206400770002", "0x2064032B0016", "0x2064032B0017", "0x2064032B0018", "0x2064032B000C", "0x2064032B000F",
            "0x2064032B000D", "0x2064032B0011", "0x2064032B000E", "0x2064032B0010", "0x206400770001", "0x2064032B0012",
            "0x2064032B0015", "0x2064032B0013", "0x2064032B0014", "0x2063032B0001", "0x2013E21C0001", "0x206500C80002",
            "0x206403430001", "0x206403430002", "0x206403430003", "0x2064032B0019", "0x206403460002", "0x206403460009",
            "0x20640346000A", "0x2064032C0019", "0x2064032C0017"));

    @Autowired
    private DeployTypeService deployTypeService;

    @PostConstruct
    private void setSelf() {
        I18nMrgUtil.getInstance().setI18nMgr(this);
        DEPLOY_I18N_MAP.put(DeployTypeEnum.E6000.getValue(), new HashMap<>());
    }

    /**
     * 取得资源文件
     *
     * @param key 资源key值
     * @param local 本地化语言
     * @return String 返回值
     */
    @Override
    public String getString(String key, Locale local) {
        if (VerifyUtil.isEmpty(key)) {
            return EMPTY_STRING;
        }

        Map<Locale, Map<String, String>> resMap = getI18nMap(key, local);
        Map<String, String> i18nMap = resMap.getOrDefault(local, Collections.emptyMap());

        if (VerifyUtil.isEmpty(i18nMap) || i18nMap.isEmpty()) {
            return DEFAULT_STRING;
        }

        return i18nMap.getOrDefault(key.trim(), DEFAULT_STRING);
    }

    /**
     * 取得资源文件，默认取英文
     *
     * @param key 资源key值
     * @return String 返回值
     */
    @Override
    public String getString(String key) {
        Locale local = new Locale(ENGLISH_LANG);
        return getString(key, local);
    }

    /**
     * 取得资源文件
     *
     * @param key 资源key值
     * @param local 本地化语言
     * @param agrs 参数
     * @return String 返回值
     */
    @Override
    public String getString(String key, Locale local, String[] agrs) {
        String oldValue = getString(key, local);
        return formatString(key, agrs, oldValue);
    }

    @Override
    public String[] getDoradoV6Args(@NonNull String alarmId, @NonNull Locale local, @NonNull String[] args) {
        if (args.length == 0) {
            return args;
        }
        String key = getParamKey(alarmId);
        Map<Locale, Map<String, String>> resMap = getI18nMap(key, local);
        Map<String, String> i18nMap = resMap.getOrDefault(local, Collections.emptyMap());

        String argsJsonStr = i18nMap.getOrDefault(key.trim(), DEFAULT_STRING);
        String[] newArgs = new String[args.length];
        try {
            JSONObject argsMap = JSON.parseObject(argsJsonStr);
            if (argsMap == null) {
                return args;
            }
            for (int i = 0; i < args.length; i++) {
                final String arg = args[i];
                String internalArg = Optional.of(i).map(String::valueOf).map(argsMap::getJSONObject)
                        .map(paramMap -> paramMap.getString(arg)).orElse(arg);
                newArgs[i] = internalArg;
            }
        } catch (JSONException | NumberFormatException e) {
            for (int i = 0; i < args.length; i++) {
                String arg = args[i];
                newArgs[i] = i18nMap.getOrDefault(arg, arg);
            }
            return newArgs;
        }
        return newArgs;
    }

    private String formatString(String key, String[] args, String oldValue) {
        String value = oldValue;
        if (!oldValue.equalsIgnoreCase(key) && args != null) {
            value = MessageFormatUtil.format(oldValue, args);
        }
        return value;
    }

    /**
     * 初始化
     */
    @Override
    public void afterPropertiesSet() {
        loadJsonI18n();
    }

    private void processUrlList(List<URL> urls, Locale locale) {
        processUrlList(urls, locale, null, null, false);
    }

    private void processUrlList(List<URL> urls, Locale locale, DeployTypeEnum deployTypeEnum,
            Set<String> excludeAlarmIds, boolean isTransformFormat) {
        if (CollectionUtils.isEmpty(urls)) {
            return;
        }
        for (URL url : urls) {
            processJsonRes(url, locale, deployTypeEnum, excludeAlarmIds, isTransformFormat);
        }
    }

    /**
     * 载入json格式国际化资源
     */
    private void loadJsonI18n() {
        processUrlList(AdapterUtils.getAllClassPathEntriesAlarmEn(), Locale.ENGLISH);
        processUrlList(AdapterUtils.getAllClassPathEntriesAlarmZh(), Locale.CHINESE);

        // 底座的告警先加载中文，再加载英文，中文比较全
        // dorado底座告警
        processUrlList(AdapterUtils.getDoradoAlarmClassPathEntriesAlarmZn(), Locale.CHINESE, null, NOT_HANDLE_ALARMS,
                true);
        processUrlList(AdapterUtils.getDoradoAlarmClassPathEntriesAlarmEn(), Locale.ENGLISH, null, NOT_HANDLE_ALARMS,
                true);
        PARAM_EXPLAIN_MAP.clear();

        // pacific底座告警
        processUrlList(AdapterUtils.getPacificAlarmClassPathEntriesAlarmZn(), Locale.CHINESE, DeployTypeEnum.E6000,
                NOT_HANDLE_ALARMS, true);
        processUrlList(AdapterUtils.getPacificAlarmClassPathEntriesAlarmEn(), Locale.ENGLISH, DeployTypeEnum.E6000,
                NOT_HANDLE_ALARMS, true);
        PARAM_EXPLAIN_MAP.clear();

        processUrlList(AdapterUtils.getAllOperationTargetClassPathEntriesAlarmEn(), Locale.ENGLISH);
        processUrlList(AdapterUtils.getAllOperationTargetClassPathEntriesAlarmZh(), Locale.CHINESE);

        processUrlList(AdapterUtils.getAnyBackUpClassPathEntriesAlarmEn(), Locale.ENGLISH);
        processUrlList(AdapterUtils.getAnyBackUpClassPathEntriesAlarmZh(), Locale.CHINESE);

        processUrlList(AdapterUtils.getSecondaryParamInternalEn(), Locale.ENGLISH);
        processUrlList(AdapterUtils.getSecondaryParamInternalZh(), Locale.CHINESE);

        PARAM_EXPLAIN_MAP.clear();
        // 加载前端定义的label
        processUrlList(AdapterUtils.getLabelExploreEn(), Locale.ENGLISH);
        processUrlList(AdapterUtils.getLabelExploreZh(), Locale.CHINESE);
        processUrlList(AdapterUtils.getLabelCommonEn(), Locale.ENGLISH);
        processUrlList(AdapterUtils.getLabelCommonZh(), Locale.CHINESE);
        processUrlList(AdapterUtils.getLabelProtectionEn(), Locale.ENGLISH);
        processUrlList(AdapterUtils.getLabelProtectionZh(), Locale.CHINESE);
        processUrlList(AdapterUtils.getLabelInsightEn(), Locale.ENGLISH);
        processUrlList(AdapterUtils.getLabelInsightZh(), Locale.CHINESE);
        processUrlList(AdapterUtils.getLabelParamsEn(), Locale.ENGLISH);
        processUrlList(AdapterUtils.getLabelParamsZh(), Locale.CHINESE);
        processUrlList(AdapterUtils.getLabelSearchEn(), Locale.ENGLISH);
        processUrlList(AdapterUtils.getLabelSearchZh(), Locale.CHINESE);
        processUrlList(AdapterUtils.getLabelSystemEn(), Locale.ENGLISH);
        processUrlList(AdapterUtils.getLabelSystemZh(), Locale.CHINESE);

        PARAM_EXPLAIN_MAP.clear();
    }

    /**
     * 处理告警文件
     *
     * @param url 路径
     * @param locale 国际化
     * @param deployTypeEnum 部署形态
     * @param excludeAlarmIds 某些不加载的告警
     * @param isTransformFormat 是否需要将告警文件的内容格式进行转化， [param]转化为{0}
     */
    @ExterAttack
    private void processJsonRes(URL url, Locale locale, DeployTypeEnum deployTypeEnum, Set<String> excludeAlarmIds,
            boolean isTransformFormat) {
        Map<Locale, Map<String, String>> resMap;
        if (deployTypeEnum == null) {
            resMap = RES_MAP;
        } else {
            resMap = DEPLOY_I18N_MAP.getOrDefault(deployTypeEnum.getValue(), RES_MAP);
        }
        try (InputStream inputStream = url.openStream()) {
            String content = IOUtils.toString(inputStream, StandardCharsets.UTF_8);
            Map<String, String> hashMap = JSON.parseObject(content, HashMap.class);
            // 排除某些告警
            excludeSomeAlarmFromFile(hashMap, excludeAlarmIds);
            String[] keyArray = hashMap.keySet().toArray(new String[0]);

            // 先要读取argument.explain，解析desc或detail时，判断[]是否是参数，需要看该参数是否在explain中定义。key排序后
            // alarm.advice、alarm.argument.explain、alarm.desc、alarm.desc.detail、alarm.effect、alarm.name
            Arrays.sort(keyArray);
            Map<String, String> data = resMap.getOrDefault(locale, new HashMap<>());
            String[] values = new String[1];
            for (String key : keyArray) {
                resolveAlarmByKey(key, values, hashMap, data, isTransformFormat);
            }
            resMap.put(locale, data);
        } catch (IOException e) {
            log.error("IO Error: ", ExceptionUtil.getErrorMessage(e));
        }
    }

    private void resolveAlarmByKey(String key, String[] values, Map<String, String> hashMap, Map<String, String> data,
            boolean isTransformFormat) {
        values[0] = hashMap.get(key);
        if (isTransformFormat && (key.contains("alarm.desc") || key.contains("alarm.advice"))) {
            String alarmId = getAlarmIdFromKey(key);
            String explainKey = alarmId + ".alarm.argument.explain";
            String paramExplain = hashMap.getOrDefault(explainKey, "");
            // 因为pacific中argument.explain英文有些没有定义，所以将使用中文中的该值
            List<String> paramExplainList = PARAM_EXPLAIN_MAP.computeIfAbsent(explainKey, tmpKey -> new ArrayList<>());
            paramExplainList.add(paramExplain);
            try {
                Map<String, Map<String, String>> paramsFromDetail = handleDescParams(values, paramExplainList);
                data.put(getParamKey(alarmId), JSON.toJSONString(paramsFromDetail));
            } catch (Exception e) {
                log.error("Load alarm occur error. alarm id is {}", alarmId, ExceptionUtil.getErrorMessage(e));
            }
        }
        data.put(key, values[0]);
    }

    private void excludeSomeAlarmFromFile(Map<String, String> hashMap, Set<String> excludeAlarmIds) {
        // 排除某些告警
        if (VerifyUtil.isEmpty(excludeAlarmIds)) {
            return;
        }
        hashMap.entrySet().removeIf(entry -> excludeAlarmIds.contains(getAlarmIdFromKey(entry.getKey())));
    }

    private Map<Locale, Map<String, String>> getI18nMap(String key, Locale locale) {
        if (key == null || locale == null) {
            return RES_MAP;
        }
        String deployType = deployTypeService.getDeployType().getValue();
        Map<Locale, Map<String, String>> i18nMap = DEPLOY_I18N_MAP.get(deployType);
        if (i18nMap == null) {
            return RES_MAP;
        }
        Map<String, String> keyMap = i18nMap.get(locale);
        if (keyMap == null) {
            return RES_MAP;
        }
        if (!keyMap.containsKey(key.trim())) {
            return RES_MAP;
        }
        return i18nMap;
    }

    private String getParamKey(String alarmId) {
        return alarmId + ".alarm.desc.param";
    }

    private String getAlarmIdFromKey(String key) {
        String[] params = key.split("\\.");
        return params[0].trim();
    }

    private Map<String, Map<String, String>> handleDescParams(String[] details, List<String> paramExplainList) {
        String detail = details[0];
        Matcher bracketMatcher = bracketPattern.matcher(detail);
        // 记录找到中括号的计数
        int bracketIndex = 0;
        Map<String, Map<String, String>> res = new HashMap<>();
        StringBuilder sb = new StringBuilder();
        int strIndex = 0;
        while (bracketMatcher.find()) {
            String bracketContent = bracketMatcher.group(1);
            if (!isParamExplainHasContent(paramExplainList, bracketContent)) {
                continue;
            }
            int bracketStart = bracketMatcher.start(1);
            int bracketEnd = bracketMatcher.end(1);
            // 找到中括号后，去匹配 []{}
            Matcher braceAndBracketMatcher = braceAndBracketPatter.matcher(detail.substring(bracketStart));
            if (braceAndBracketMatcher.find()) {
                bracketEnd = braceAndBracketMatcher.end(1) + bracketStart;
                // 大括号的内容， eg: {0:backup;1:restore;2:archive}
                String braceContent = braceAndBracketMatcher.group(1);
                fillBraceContent(res, braceContent, bracketIndex);
            }
            // 这里将参数[]或[]{}替换为{0}。统一成备份软件的参数定义形式，以便后续formatString统一解析。
            sb.append(detail, strIndex, bracketStart);
            sb.append("{");
            int paramIndex = findParamIndex(paramExplainList, bracketContent);
            if (paramIndex != -1) {
                sb.append(paramIndex);
            } else {
                sb.append(bracketIndex);
            }
            sb.append("}");
            strIndex = bracketEnd;
            bracketIndex++;
        }
        sb.append(detail.substring(strIndex));
        details[0] = sb.toString();
        return res;
    }

    private int findParamIndex(List<String> paramExplainList, String contents) {
        String[] regexKey = {"\\", "$", "(", ")", "*", "+", ".", "[", "]", "?", "^", "{", "}", "|"};
        String content = contents;
        for (String key : regexKey) {
            if (content.contains(key)) {
                content = content.replace(key, "\\" + key);
            }
        }
        Pattern indexPatter = Pattern.compile("[0-9]+(?=\\." + content + ")");
        for (String paramExplain : paramExplainList) {
            Matcher indexMatcher = indexPatter.matcher(paramExplain);
            if (indexMatcher.find()) {
                return Integer.parseInt(indexMatcher.group(0)) - 1;
            }
        }
        return -1;
    }

    private boolean isParamExplainHasContent(List<String> paramExplainList, String content) {
        for (String paramExplain : paramExplainList) {
            if (paramExplain.contains(content)) {
                return true;
            }
        }
        return false;
    }

    private void fillBraceContent(Map<String, Map<String, String>> res, String braceContent, int index) {
        Map<String, String> contentMap = new HashMap<>();

        String innerContent = braceContent.substring(1, braceContent.length() - 1);
        String[] keyValues = innerContent.split(";");
        for (String keyValue : keyValues) {
            String[] params = keyValue.trim().split(":");
            if (params.length == 2) {
                contentMap.put(params[0].trim(), params[1].trim());
            }
        }
        res.put(String.valueOf(index), contentMap);
    }
}
