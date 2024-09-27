package openbackup.access.framework.resource.persistence.dao;

import openbackup.access.framework.resource.persistence.model.ResourceDesesitizationPo;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Mapper;
import org.springframework.stereotype.Component;

/**
 * Resource Desesitization Mapper
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-04-08
 */
@Mapper
@Component
public interface ResourceDesesitizationMapper extends BaseMapper<ResourceDesesitizationPo> {
}
