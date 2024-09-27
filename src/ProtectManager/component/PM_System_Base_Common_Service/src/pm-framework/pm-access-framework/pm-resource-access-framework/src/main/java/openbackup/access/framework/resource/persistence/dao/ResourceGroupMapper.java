package openbackup.access.framework.resource.persistence.dao;

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;
import openbackup.access.framework.resource.persistence.model.ResourceGroupExtendField;
import openbackup.access.framework.resource.persistence.model.ResourceGroupPo;

import com.baomidou.mybatisplus.core.conditions.Wrapper;
import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.baomidou.mybatisplus.core.metadata.IPage;

import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Select;
import org.springframework.stereotype.Repository;

import java.util.List;

/**
 * 资源组dao
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-1-18
 */
@Repository
public interface ResourceGroupMapper extends BaseMapper<ResourceGroupPo> {
    /**
     * 联表查询语句
     */
    String JOIN_SQL = "select g.*, p.sla_name, p.sla_compliance as is_sla_compliance "
            + "from t_resource_group as g "
            + "left join protected_object as p on g.uuid = p.uuid ";

    /**
     * 分页查询语句
     */
    String PAGE_SQL = "select * from ( " + JOIN_SQL + " ) as q ${ew.customSqlSegment}";

    /**
     * selectByName
     *
     * @param scopeResourceId scopeResourceId
     * @param name name
     * @return List<ResourceGroupPo>
     */
    @ExterAttack
    List<ResourceGroupPo> selectByScopeResourceIdAndName(
            @Param("scopeResourceId") String scopeResourceId, @Param("name") String name);

    /**
     * page
     *
     * @param page page
     * @param queryWrapper queryWrapper
     * @return IPage<ResourceGroupExtendField>
     */
    @Select(PAGE_SQL)
    IPage<ResourceGroupExtendField> page(IPage<ResourceGroupExtendField> page,
                                         @Param("ew") Wrapper<ResourceGroupExtendField> queryWrapper);
}