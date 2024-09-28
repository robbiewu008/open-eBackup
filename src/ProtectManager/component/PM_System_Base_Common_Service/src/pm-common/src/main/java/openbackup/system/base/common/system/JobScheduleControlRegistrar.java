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
package openbackup.system.base.common.system;

import openbackup.system.base.common.annotation.JobScheduleControl;
import openbackup.system.base.common.annotation.JobScheduleControls;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.CommonUtil;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.api.JobCenterNativeApi;
import openbackup.system.base.sdk.job.model.request.JobScheduleConfig;
import openbackup.system.base.sdk.job.model.request.JobScheduleRule;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeansException;
import org.springframework.beans.factory.config.BeanPostProcessor;
import org.springframework.boot.CommandLineRunner;
import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.core.annotation.AnnotationUtils;
import org.springframework.core.env.ConfigurableEnvironment;
import org.springframework.stereotype.Component;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import java.io.File;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * Job Schedule Control Registrar
 *
 */
@Component
@Slf4j
public class JobScheduleControlRegistrar implements BeanPostProcessor, CommandLineRunner {
    private final ConcurrentLinkedQueue<JobScheduleConfig> jobScheduleConfigs = new ConcurrentLinkedQueue<>();

    private final ConfigurableEnvironment environment;

    private final JobCenterNativeApi jobCenterNativeApi;

    public JobScheduleControlRegistrar(ConfigurableEnvironment environment, JobCenterNativeApi jobCenterNativeApi) {
        this.environment = environment;
        this.jobCenterNativeApi = jobCenterNativeApi;
    }

    /**
     * 注册任务调度控制配置
     *
     * @param bean bean
     * @param beanName bean name
     * @return bean object
     * @throws BeansException beans exception
     */
    @Override
    public Object postProcessAfterInitialization(Object bean, String beanName) throws BeansException {
        Collection<JobScheduleConfig> collection = getJobScheduleControlConfig(bean.getClass());
        if (!collection.isEmpty()) {
            log.info("found job schedule control config for bean: {}", beanName);
            jobScheduleConfigs.addAll(collection);
        }
        return bean;
    }

    private Collection<JobScheduleConfig> getJobScheduleControlConfig(Class<?> clazz) {
        Map<String, JobScheduleConfig> jobScheduleConfigMap = new HashMap<>();
        resolveJobScheduleControlConfigs(jobScheduleConfigMap, clazz);
        Method[] methods = clazz.getMethods();
        RequestMapping clazzRequestMapping = AnnotationUtils.getAnnotation(clazz, RequestMapping.class);
        if (clazzRequestMapping == null) {
            return Collections.emptyList();
        }
        for (Method method : methods) {
            JobScheduleControl control = AnnotationUtils.getAnnotation(method, JobScheduleControl.class);
            if (control == null) {
                continue;
            }
            String[] paths = getRequestPaths(clazzRequestMapping, method);
            for (String path : paths) {
                JobScheduleRule rule = parseJobScheduleRule(control, path);
                getOrNewJobScheduleConfig(jobScheduleConfigMap, control).getRules().add(rule);
            }
        }
        return jobScheduleConfigMap.values();
    }

    private JobScheduleRule parseJobScheduleRule(JobScheduleControl control, String path) {
        JobScheduleRule rule = new JobScheduleRule();
        String scope = control.scope();
        if (!scope.isEmpty()) {
            if (scope.startsWith("{")) {
                rule.setScope(JSONObject.fromObject(scope));
            } else if (scope.startsWith("[")) {
                rule.setScope(JSONArray.fromObject(scope));
            } else {
                rule.setScope(scope);
            }
        }
        buildGlobalJobLimit(rule, control.globalJobLimit());
        rule.setScopeJobLimit(control.scopeJobLimit());
        rule.setMajorPriority(control.majorPriority());
        if (control.minorPriorities().length > 0) {
            rule.setMinorPriorities(Arrays.asList(control.minorPriorities()));
        }
        rule.setStrictScope(control.strictScope());
        rule.setResumeStatus(control.resumeStatus());
        rule.setExamine(path);
        return rule;
    }

    /**
     * 构造globalJobLimit
     *
     * jobLimit中的value的格式存在两种形式：
     * 1. 数字
     * 2. 字符串
     * 当为数字，或者字符串形式的数字时，则可直接作为limit值
     * 当为非数字格式字符串则需要从环境变量中取对应的值，此时环境变量中对应值不存在或不为数字形式
     *
     * @param rule 任务调度规则
     * @param jobLimit 任务限制配置
     */
    private void buildGlobalJobLimit(JobScheduleRule rule, String jobLimit) {
        if (VerifyUtil.isEmpty(jobLimit)) {
            return;
        }
        JSONObject globalJobLimit = JSONObject.fromObject(jobLimit);
        JSONObject config = new JSONObject();
        for (Object key : globalJobLimit.keySet()) {
            Object value = globalJobLimit.get(key);
            Object configValue = value;
            if (value instanceof Integer) {
                config.put(key, configValue);
                continue;
            }
            if (StringUtils.isNumeric(value.toString())) {
                configValue = Integer.valueOf(value.toString());
            } else {
                String property = environment.getProperty(value.toString());
                if (!StringUtils.isNumeric(property)) {
                    log.info("Config: {} property: {} not an integer.", key.toString(), property);
                    return;
                }
                configValue = Integer.valueOf(property);
            }
            config.put(key, configValue);
        }
        rule.setGlobalJobLimit(config.toMap(Integer.class));
    }

    private JobScheduleConfig getOrNewJobScheduleConfig(Map<String, JobScheduleConfig> jobScheduleConfigMap,
            JobScheduleControl control) {
        String jobType = control.jobType().getValue();
        return Optional.ofNullable(jobScheduleConfigMap.get(jobType)).orElseGet(() -> {
            JobScheduleConfig scheduleConfig = new JobScheduleConfig();
            scheduleConfig.setJobType(jobType);
            scheduleConfig.setRules(new ArrayList<>());
            jobScheduleConfigMap.put(jobType, scheduleConfig);
            return scheduleConfig;
        });
    }

    private void resolveJobScheduleControlConfigs(Map<String, JobScheduleConfig> jobScheduleConfigMap, Class<?> clazz) {
        Set<JobScheduleControl> controls = AnnotatedElementUtils.findMergedRepeatableAnnotations(clazz,
                JobScheduleControl.class, JobScheduleControls.class);
        for (JobScheduleControl control : controls) {
            JobScheduleRule rule = parseJobScheduleRule(control, null);
            getOrNewJobScheduleConfig(jobScheduleConfigMap, control).getRules().add(rule);
        }
    }

    private String[] getRequestPaths(RequestMapping clazzRequestMapping, Method method) {
        RequestMapping methodRequestMapping = AnnotationUtils.getAnnotation(method, RequestMapping.class);
        if (methodRequestMapping == null) {
            log.error("method: {} is not rest api method.", method);
            return new String[0];
        }
        if (!Arrays.asList(methodRequestMapping.method()).contains(RequestMethod.POST)) {
            log.error("http method: {} should be post.", method);
            return new String[0];
        }
        return buildServicePath(clazzRequestMapping.path(), methodRequestMapping.path());
    }

    private String[] buildServicePath(String[] prefixes, String[] suffixes) {
        if (prefixes.length == 0) {
            return suffixes;
        }
        if (suffixes.length == 0) {
            return prefixes;
        }
        List<String> items = new ArrayList<>();
        for (String prefix : prefixes) {
            String path0 = prefix.replaceAll("/\\s*$", "");
            for (String suffix : suffixes) {
                if (suffix.isEmpty()) {
                    items.add(prefix);
                    continue;
                }
                String path1 = suffix.replaceAll("^\\s*/", "");
                items.add(path0 + File.separator + path1);
            }
        }
        return items.toArray(new String[0]);
    }

    private void updateJobSchedulePolicy(JobScheduleConfig config) {
        for (int i = 0; i < IsmNumberConstant.THOUSAND; i++) {
            try {
                jobCenterNativeApi.updateJobSchedulePolicy(config);
                log.info("update job schedule policy success.");
                return;
            } catch (RuntimeException e) {
                log.error("update job schedule policy failed.", ExceptionUtil.getErrorMessage(e));
                CommonUtil.sleep(IsmNumberConstant.THOUSAND);
            }
        }
        log.error("update job schedule policy failed after retry 1000 times.");
    }

    @Override
    public void run(String... args) throws Exception {
        jobScheduleConfigs.forEach(this::updateJobSchedulePolicy);
    }
}
