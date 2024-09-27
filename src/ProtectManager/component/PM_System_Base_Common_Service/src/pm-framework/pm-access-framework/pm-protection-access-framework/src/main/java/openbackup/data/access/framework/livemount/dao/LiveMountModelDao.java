package openbackup.data.access.framework.livemount.dao;

import openbackup.data.access.framework.livemount.common.model.LiveMountModel;

import com.baomidou.mybatisplus.core.conditions.Wrapper;
import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.baomidou.mybatisplus.core.metadata.IPage;

import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Select;
import org.springframework.stereotype.Component;

/**
 * Live Mount Model Dao
 *
 * @author l00272247
 * @since 2020-09-26
 */
@Mapper
@Component
public interface LiveMountModelDao extends BaseMapper<LiveMountModel> {
    /**
     * JOIN_SQL
     */
    String JOIN_SQL = "select a.*, b.name as policy_name, d.cluster_name as cluster_name from live_mount as a "
            + "left join live_mount_policy as b on a.policy_id = b.policy_id "
            + "left join copies as c on a.copy_id = c.uuid "
            + "left join t_cluster_member as d on c.device_esn = d.remote_esn";

    /**
     * WRAP_SQL
     */
    String WRAP_SQL = "SELECT * from ( " + JOIN_SQL + " ) AS q ${ew.customSqlSegment}";

    /**
     * page query
     *
     * @param page page
     * @param queryWrapper query wrapper
     * @return page
     */
    @Select(WRAP_SQL)
    IPage<LiveMountModel> page(IPage<LiveMountModel> page, @Param("ew") Wrapper<LiveMountModel> queryWrapper);
}
