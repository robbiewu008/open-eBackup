/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.service;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.security.EncryptorUtil;
import openbackup.system.base.sdk.auth.model.dao.PresetAccountDao;
import openbackup.system.base.sdk.auth.model.dao.PresetAccountPo;
import openbackup.system.base.sdk.auth.model.request.PresetAccountRequest;
import openbackup.system.base.sdk.common.model.UuidObject;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

/**
 * 预置账号处理
 *
 * @author y30021475
 * @since 2023-08-07
 */
@Slf4j
@Service
public class PresetAccountServiceImpl {
    private final PresetAccountDao presetAccountDao;

    private final EncryptorUtil encryptorUtil;

    /**
     * 构造
     *
     * @param presetAccountDao presetAccountDao
     * @param encryptorUtil encryptorUtil
     */
    public PresetAccountServiceImpl(PresetAccountDao presetAccountDao, EncryptorUtil encryptorUtil) {
        this.presetAccountDao = presetAccountDao;
        this.encryptorUtil = encryptorUtil;
    }

    /**
     * 保存预置账号密码
     *
     * @param presetAccountRequest presetAccountRequest
     * @return UuidObject
     */
    public UuidObject savePresetAccountAndPwd(PresetAccountRequest presetAccountRequest) {
        PresetAccountPo presetAccountPo = new PresetAccountPo();
        try {
            LambdaQueryWrapper<PresetAccountPo> wrapper = new LambdaQueryWrapper<>();
            wrapper.eq(PresetAccountPo::getUserName, presetAccountRequest.getUserName());
            wrapper.eq(PresetAccountPo::getSourceType, presetAccountRequest.getSourceType());
            presetAccountDao.delete(wrapper);
            presetAccountPo.setUserName(presetAccountRequest.getUserName());
            presetAccountPo.setUserPwd(encryptorUtil.getEncryptPwd(presetAccountRequest.getUserPwd()));
            presetAccountPo.setSourceType(presetAccountRequest.getSourceType());
            String uuid = UUIDGenerator.getUUID();
            presetAccountPo.setUuid(uuid);
            presetAccountDao.insert(presetAccountPo);
            return new UuidObject(uuid);
        } finally {
            log.debug("start to clean preset account!");
            StringUtil.clean(presetAccountRequest.getUserPwd());
            StringUtil.clean(presetAccountPo.getUserPwd());
        }
    }

    /**
     * 查询预置账号密码
     *
     * @param userName userName
     * @param sourceType sourceType
     * @return PresetAccountPo
     */
    public PresetAccountPo queryPresetAccountAndPwdByNameAndType(String userName, String sourceType) {
        LambdaQueryWrapper<PresetAccountPo> wrapper = new LambdaQueryWrapper<>();
        wrapper.eq(PresetAccountPo::getUserName, userName);
        wrapper.eq(PresetAccountPo::getSourceType, sourceType);
        PresetAccountPo presetAccountPo = presetAccountDao.selectOne(wrapper);
        if (VerifyUtil.isEmpty(presetAccountPo)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "the user not exist!");
        }
        presetAccountPo.setUserPwd(encryptorUtil.getDecryptPwd(presetAccountPo.getUserPwd()));
        return presetAccountPo;
    }
}
