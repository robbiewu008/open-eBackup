package openbackup.access.framework.resource.persistence.dao;

import openbackup.access.framework.resource.persistence.model.ProtectedEnvironmentExtendInfoPo;
import openbackup.system.base.security.callee.CalleeMethod;
import openbackup.system.base.security.callee.CalleeMethods;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Mapper;
import org.springframework.stereotype.Component;

/**
 * Protected Environment Extend Info Mapper
 *
 * @author l00272247
 * @since 2021-10-18
 */
@Mapper
@Component
@CalleeMethods(
    name = "environment_dao",
    value = {@CalleeMethod(name = "selectById")})
public interface ProtectedEnvironmentExtendInfoMapper extends BaseMapper<ProtectedEnvironmentExtendInfoPo> {
}
