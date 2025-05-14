/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.controller;

import static openbackup.system.base.common.constants.CommonErrorCode.STATUS_ERROR;
import static openbackup.system.base.common.constants.InitConstants.INITIAL_ESN;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants;
import com.huawei.oceanprotect.system.base.dto.pacific.NetworkInfoDto;
import com.huawei.oceanprotect.system.base.dto.pacific.NodeNetworkInfoDto;
import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigStatus;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InitServiceType;
import com.huawei.oceanprotect.system.base.initialize.status.InitStandardBackupService;
import com.huawei.oceanprotect.system.base.initialize.status.InitStatusService;
import com.huawei.oceanprotect.system.base.service.InitConfigService;
import com.huawei.oceanprotect.system.base.service.SystemService;
import com.huawei.oceanprotect.system.base.service.impl.InitNetworkServiceImpl;
import com.huawei.oceanprotect.system.base.user.common.utils.CurrentSystemTime;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.bean.DeviceUser;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.CommonOperationCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.enums.AuthTypeEnum;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.enums.ServiceType;
import openbackup.system.base.common.exception.DeviceManagerException;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.log.constants.EventTarget;
import openbackup.system.base.common.process.ProcessException;
import openbackup.system.base.common.process.ProcessUtil;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.sdk.system.model.EsnInfo;
import openbackup.system.base.sdk.system.model.StorageAuth;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.journal.Loggings;
import openbackup.system.base.security.permission.Permission;
import openbackup.system.base.util.ProviderRegistry;

import org.apache.commons.lang3.StringUtils;
import org.hibernate.validator.constraints.Length;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.LocalTime;
import java.time.ZoneOffset;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Optional;
import java.util.TimeZone;
import java.util.UUID;

import javax.validation.Valid;
import javax.validation.constraints.Pattern;

/**
 * 系统控制器，提供后台相关RESTful 接口
 *
 * @author q50016449
 * @version [OceanStor 100P 8.1.0]
 * @since 2020-09-29
 */
@Slf4j
@RestController
@RequestMapping("/v1/system")
public class SystemController {
    /**
     * 亚洲/上海 时区
     */
    private static final String TIME_ZONE_SHANGHAI = "Asia/Shanghai";

    @Autowired
    private InitNetworkConfigMapper initNetworkConfigMapper;

    @Autowired
    private InitStatusService initStatusService;

    @Autowired
    private InitStandardBackupService initStandardBackupService;

    @Autowired
    private ProviderRegistry registry;

    @Autowired
    private SystemService systemService;

    /**
     * Redis客户端
     */
    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private ClusterQueryService clusterQueryService;

    @Autowired
    private InitConfigService initConfigService;

    @Autowired
    private ClusterBasicService clusterBasicService;

    /**
     * 提供后台时间，时区等信息接口
     *
     * @return CurrentSystemTime
     */
    @ExterAttack
    @GetMapping("/time")
    @Permission(roles = {
        Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN, Constants.Builtin.ROLE_AUDITOR,
        Constants.Builtin.ROLE_DEVICE_MANAGER}, enableCheckAuth = false)
    public CurrentSystemTime getSystemTime() {
        TimeZone.setDefault(null);
        System.setProperty("user.timezone", "");
        CurrentSystemTime systemTime = new CurrentSystemTime();
        // 获取jvm系统日历
        Calendar calendar = Calendar.getInstance();
        // 获取jvm系统时间, 无夏令时
        Date date = calendar.getTime();
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        // 获取jvm时区
        TimeZone defaultTime = TimeZone.getDefault();
        dateFormat.setTimeZone(TimeZone.getTimeZone(TimeZone.getDefault().getID()));
        systemTime.setOffset(defaultTime.getOffset(date.getTime()));
        // 获取linux系统时间，包含夏令时
        String[] res = getLinuxSystemTime();
        // UTC+GMT时间偏移量存储变量
        String timezoneId;
        DateFormat timeZoneDateFormat = new SimpleDateFormat("Z");
        // 对比linux系统时区偏移和jvm时区偏移
        String standardOffset = getStandarfOffset(defaultTime.getOffset(date.getTime()));
        if (ZoneOffset.of(res[2]).equals(ZoneOffset.of(standardOffset))) {
            systemTime.setDstSavings(0);
            systemTime.setUseDaylight(false);
            timezoneId = timeZoneDateFormat.format(calendar.getTime());
        } else {
            systemTime.setDstSavings(LegoNumberConstant.ONE);
            systemTime.setUseDaylight(true);
            calendar.add(Calendar.HOUR, 1);
            timezoneId = daylightSavingsDeal(timeZoneDateFormat, calendar);
            systemTime.setOffset(ZoneOffset.of(res[2]).getTotalSeconds() * 1000);
        }
        // GMT格式
        StringBuilder idString = new StringBuilder("GMT" + timezoneId);
        idString.insert(LegoNumberConstant.SIX, ":");
        systemTime.setId(idString.toString());
        // 当前linux时间, 包含夏令时
        String[] dateInfo = res[0].split("-");
        String[] timeInfo = res[1].split(":");
        LocalDateTime systemDstSavingTime = LocalDateTime.of(
            LocalDate.of(Integer.parseInt(dateInfo[0]), Integer.parseInt(dateInfo[1]), Integer.parseInt(dateInfo[2])),
            LocalTime.of(Integer.parseInt(timeInfo[0]), Integer.parseInt(timeInfo[1]), Integer.parseInt(timeInfo[2])));
        date = Date.from(systemDstSavingTime.toInstant(ZoneOffset.of(res[2])));
        systemTime.setTime(dateFormat.format(date));
        // UTC格式
        StringBuilder nameString = new StringBuilder("UTC" + timezoneId);
        nameString.insert(LegoNumberConstant.SIX, ":");
        systemTime.setDisplayName(nameString.toString());
        return systemTime;
    }

    private String getStandarfOffset(int offsetnano) {
        String prefix = offsetnano >= 0 ? "+" : "-";
        String offsetHour;
        if (offsetnano == 0) {
            offsetHour = "0000";
        } else if (Math.abs(offsetnano / 36000) < 1000) {
            offsetHour = "0" + Math.abs(offsetnano / 36000);
        } else {
            offsetHour = String.valueOf(Math.abs(offsetnano / 36000));
        }
        return prefix + offsetHour;
    }

    private String[] getLinuxSystemTime() {
        List<String> cmd = new ArrayList<>();
        cmd.add("date");
        cmd.add("+'%Y-%m-%d %H:%M:%S %:z'");
        try {
            return ProcessUtil.executeInSeconds(cmd, 3).getOutputList().get(0)
                .replaceAll("'", "").split(" ");
        } catch (ProcessException e) {
            log.error("command execute error, command:" + cmd);
            throw new LegoCheckedException(STATUS_ERROR, "command execute error");
        }
    }

    private String daylightSavingsDeal(DateFormat timeZoneDateFormat, Calendar calendar) {
        String tempId = timeZoneDateFormat.format(calendar.getTime()).substring(1);
        String startIndex = timeZoneDateFormat.format(calendar.getTime()).substring(0, 1);
        // 根据符号设定偏移值
        int offset = 100;
        if (startIndex.contains("-")) {
            offset = -100;
        }
        String timeIndexString = "" + (Integer.parseInt(tempId) + offset);

        if (timeIndexString.length() == 1) {
            return "+0000";
        }
        if (timeIndexString.length() < 4) {
            timeIndexString = "0" + timeIndexString;
        }
        return startIndex + timeIndexString;
    }

    /**
     * 是否需要初始化
     *
     * @return 是否初始化
     */
    @ExterAttack
    @GetMapping("/initConfig")
    @Permission(
        roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN, Constants.Builtin.ROLE_AUDITOR},
        enableCheckAuth = false)
    public ConfigStatus getInitConfig() {
        // 初始化结果
        ConfigStatus status = new ConfigStatus();
        status.setStatus(InitConfigConstant.ERROR_CODE_YES);

        RMap<String, String> statusMap = redissonClient.getMap(InitConfigConstant.INIT_RUNNING_FLAG);
        String modifyStatus = statusMap.get(InitConfigConstant.INIT_STAUS_FLAG);
        log.info("get initialize status info is modifyStatus: {}", modifyStatus);

        // 从数据库中查询状态码“INIT_ERROR_FLAG”
        List<InitConfigInfo> statusCode = initNetworkConfigMapper.queryInitConfig(Constants.INIT_ERROR_FLAG);
        if (statusCode == null || statusCode.size() == 0) {
            // 为空则说明：在初始化中
            if (!VerifyUtil.isEmpty(modifyStatus)) {
                status.setStatus(InitConfigConstant.ERROR_CODE_RUNNING);
            }
            return status;
        }
        if (isInitOverTime(statusCode)) {
            status.setStatus(InitConfigConstant.ERROR_CODE_FAILED);
            return status;
        }
        // 从数据库中查询状态码“BACKUP_NETWORK_FLAG”
        List<InitConfigInfo> initConfig = initNetworkConfigMapper.queryInitConfig(Constants.BACKUP_NETWORK_FLAG);
        if (initConfig != null && initConfig.size() > 0) {
            // 不为空则说明：不需要初始化
            status.setStatus(InitConfigConstant.ERROR_CODE_NO);
            return status;
        }

        // 状态码“BACKUP_NETWORK_FLAG”为空，但是状态码“INIT_ERROR_FLAG”不为空
        int initValue = Integer.parseInt(statusCode.get(0).getInitValue());
        log.info("current init status value is status: {}", initValue);
        if (initValue == InitConfigConstant.ERROR_CODE_NO || initValue == Constants.ERROR_CODE_OK) {
            // 不需要初始化，或者已经完成
            status.setStatus(InitConfigConstant.ERROR_CODE_NO);
            return status;
        }
        if (initValue == InitConfigConstant.ERROR_CODE_UNRECOVERABLE_FAILED) {
            // 不可回退状态，直接展示错误信息
            status.setStatus(InitConfigConstant.ERROR_CODE_UNRECOVERABLE_FAILED);
            status.setCode(String.valueOf(CommonErrorCode.INITALIZATION_UNRECOVERABLE_EXCEPTION));
            return status;
        }
        if (initValue == InitConfigConstant.ERROR_CODE_RUNNING) {
            if (VerifyUtil.isEmpty(modifyStatus)) {
                status.setStatus(InitConfigConstant.ERROR_CODE_YES);
                return status;
            }

            // 正在处理
            status.setStatus(InitConfigConstant.ERROR_CODE_RUNNING);
            return status;
        }
        return status;
    }

    private boolean isInitOverTime(List<InitConfigInfo> statusCode) {
        if (isInitRunning(statusCode)) {
            long lastInitTime = statusCode.get(0).getCreateTime();
            return System.currentTimeMillis() - lastInitTime > InitNetworkConfigConstants.INIT_OVER_TIME;
        }
        return false;
    }

    private static boolean isInitRunning(List<InitConfigInfo> statusCode) {
        return StringUtils.equals(statusCode.get(0).getInitValue(),
                String.valueOf(InitConfigConstant.ERROR_CODE_RUNNING));
    }

    /**
     * 内部调用
     *
     * @return 是否初始化
     */
    @ExterAttack
    @GetMapping("/internal/initConfig")
    public ConfigStatus getInitConfigInternal() {
        return getInitConfig();
    }

    /**
     * 内部调用
     *
     * @return 时区信息
     */
    @ExterAttack
    @GetMapping("/internal/timezone")
    public String getTimeZoneInfo() {
        String timeZone = TimeZone.getDefault().getID();
        if (VerifyUtil.isEmpty(timeZone)) {
            timeZone = TIME_ZONE_SHANGHAI;
        }
        return timeZone;
    }

    /**
     * 刷新初始化状态
     *
     * @return 状态
     */
    @ExterAttack
    @GetMapping("/initStatusInfo")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    public ConfigStatus getInitStatusInfo() {
        return initStatusService.getInitConfigStatus();
    }

    /**
     * 更新数据库状态
     *
     */
    @ExterAttack
    @PutMapping("/status-info")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    @Logging(name = CommonOperationCode.UPDATE_STATUS_INFO, target = "System")
    public void updateStatusInfo() {
        if (initStatusService.isInitFinish()) {
            log.info("update init status");
            initStatusService.updateInitStatus();
            return;
        }
        log.info("no need update init status");
    }

    /**
     * 创建初始化网络
     *
     * @param initNetworkBody 初始化配置 请求体
     * @return 是否初始化成功
     */
    @ExterAttack
    @PostMapping("/initConfig")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    @Loggings(value = {
        @Logging(name = CommonOperationCode.CREATE_X_SERIES_INITCONFIG,
        rank = FaultEnum.AlarmSeverity.INFO,
        target = EventTarget.SYSTEM,
        details = {"@init_log_params_service_get_storage_auth_name.call($1)",
            "@init_log_params_service_get_backup_network_name.call($1)",
            "@init_log_params_service_get_copy_network_name.call($1)",
            "@init_log_params_service_get_archive_network_name.call($1)"},
        deployType = {DeployTypeEnum.X3000, DeployTypeEnum.X6000,
            DeployTypeEnum.X8000, DeployTypeEnum.X9000}),
        @Logging(name = CommonOperationCode.CREATE_E6000_INITCONFIG, rank = FaultEnum.AlarmSeverity.INFO,
            target = EventTarget.SYSTEM,
            details = {"@init_log_params_service_get_backup_network_info_from_request.call($1)",
                "@init_log_params_service_get_archive_network_info_from_request.call($1)"},
            deployType = {DeployTypeEnum.E6000}),
        @Logging(name = CommonOperationCode.CREATE_DEPENDENT_INITCONFIG, rank = FaultEnum.AlarmSeverity.INFO,
            target = EventTarget.SYSTEM,
            details = {"@init_log_params_service_get_backup_network_info_from_request.call($1)",
                "@init_log_params_service_get_archive_network_info_from_request.call($1)",
                "@init_log_params_service_get_copy_network_info_from_request.call($1)"},
            deployType = {DeployTypeEnum.E1000})})
    public ConfigStatus createInitConfig(@RequestBody @Valid InitNetworkBody initNetworkBody) {
        log.info("Receive the initialize backup config message");
        ConfigStatus status = new ConfigStatus();
        try {
            status.setStatus(InitConfigConstant.ERROR_CODE_RUNNING);
            InitNetworkServiceImpl provider = registry.findProvider(InitNetworkServiceImpl.class,
                InitServiceType.INIT_NETWORK.getType());
            String initStatus = provider.init(initNetworkBody);
            if (InitConfigConstant.INIT_READY_SUCCESS.equals(initStatus)) {
                status.setStatus(Constants.ERROR_CODE_OK);
                return status;
            }
        } catch (DeviceManagerException e) {
            log.error("create init config error, error code: {}, error desc: {}", e.getCode(), e.getDesc());
            throw e.toLegoException();
        }
        return status;
    }

    /**
     * 查询服务进度
     *
     * @param serviceType 查询服务请求类型
     * @return 状态
     */
    @ExterAttack
    @GetMapping("/service-status-info")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        enableCheckAuth = false)
    public ConfigStatus initServiceStatus(@RequestParam("serviceType") ServiceType serviceType) {
        return initStandardBackupService.getInitConfigStatus(serviceType);
    }

    /**
     * 获取pacific 多节点下的业务网络配置信息
     *
     * @param manageIp 节点的管理ip
     * @return 业务网络配置信息
     */
    @ExterAttack
    @GetMapping("/network-info")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    public NetworkInfoDto getNetworkInfo(@Length(min = 1, max = 64)
    @RequestParam(value = "manageIp", required = false) String manageIp) {
        return systemService.getNetworkInfo(manageIp);
    }

    /**
     * 获取pacific 指定节点下的业务网络配置信息
     *
     * @param manageIp 节点的管理ip
     * @param ifaceName 端口
     * @param ipAddress ip地址和掩码
     * @return 业务网络配置信息
     */
    @ExterAttack
    @GetMapping("/node-network-info")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    public NodeNetworkInfoDto getNodeNetworkInfo(@Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS,
        message = "cluster ip is invalid, not ipv4 or ipv6.") @RequestParam(value = "manageIp") String manageIp,
        @Length(min = 1, max = 64) @RequestParam(value = "ifaceName", required = false) String ifaceName,
        @Length(min = 1, max = 64) @RequestParam(value = "ipAddress", required = false) String ipAddress) {
        return systemService.getNodeNetworkInfo(manageIp, ifaceName, ipAddress);
    }

    /**
     * 配置存储认证
     *
     * @param storageAuth 用户信息
     */
    @ExterAttack
    @PostMapping("/auth")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    @Logging(name = CommonOperationCode.INIT_CONFIG_STORAGE_AUTH, rank = FaultEnum.AlarmSeverity.INFO,
        target = EventTarget.SYSTEM, details = {"$1.username"})
    public void configStorageAuth(@RequestBody @Valid StorageAuth storageAuth) {
        try {
            String randomDeviceId = UUID.randomUUID().toString();
            DeviceUser deviceUser = new DeviceUser();
            deviceUser.setUsername(storageAuth.getUsername());
            deviceUser.setPassword(storageAuth.getPassword());
            deviceUser.setAuthType(AuthTypeEnum.MANAGER_AUTH.getEntryAuthType());
            deviceUser.setId(randomDeviceId);
            String deviceId = systemService.configStorageAuth(deviceUser);
            redissonClient.getBucket(INITIAL_ESN).set(deviceId);
            initConfigService.updateLocalStorageDeviceId(deviceId);
            // 将存储认证信息保存到数据库中
            initConfigService.updateLocalStorageAuth(storageAuth);
        } catch (DeviceManagerException e) {
            log.error("Get storage auth config error, error code: {}, error desc: {}", e.getCode(), e.getDesc());
            throw e.toLegoException();
        } finally {
            Optional.ofNullable(storageAuth).ifPresent(a -> StringUtil.clean(a.getPassword()));
        }
    }

    /**
     * 查询集群esn
     *
     * @return esn
     */
    @GetMapping("/esn")
    @ExterAttack
    @Permission(roles = {
            Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_AUDITOR, Constants.Builtin.ROLE_RD_ADMIN
    }, enableCheckAuth = false, checkRolePermission = true)
    public EsnInfo queryEsn() {
        return new EsnInfo(clusterBasicService.getCurrentClusterEsn());
    }
}
