package openbackup.data.access.framework.core.dao;

import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.system.base.security.exterattack.ExterAttack;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.springframework.stereotype.Component;

/**
 * Protected Object Mapper
 *
 * @author l00272247
 * @since 2021-10-20
 */
@Mapper
@Component
public interface ProtectedObjectMapper extends BaseMapper<ProtectedObjectPo> {
    /**
     * 根据SLA的ID查询关联资源数量【内部接口】
     *
     * @param slaName SLA的Name
     * @param userId 用户Id
     * @param subType 资源子类型
     * @return 数量
     */
    int countBySubTypeAndSlaName(@Param("slaName") String slaName, @Param("userId") String userId,
            @Param("subType") String subType);

    /**
     * 根据SLA的ID查询关联资源数量【内部接口】
     *
     * @param slaId slaId
     * @return 数量
     */
    int countBySlaId(@Param("slaId") String slaId);

    /**
     * 手动保存保护对象
     *
     * @param protectedObjectPo protectedObjectPo
     * @return 数量
     */
    @ExterAttack
    int insertProtectedObject(@Param("po") ProtectedObjectPo protectedObjectPo);

    /**
     * 更新json 类型的 extParameters
     *
     * @param protectedObjectPo  protectedObjectPo
     */
    @ExterAttack
    void updateExtParameters(@Param("po") ProtectedObjectPo protectedObjectPo);
}
