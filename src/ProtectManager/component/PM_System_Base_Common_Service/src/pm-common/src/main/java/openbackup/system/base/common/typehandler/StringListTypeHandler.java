package openbackup.system.base.common.typehandler;

import com.alibaba.fastjson.TypeReference;

import org.apache.ibatis.type.MappedTypes;

import java.util.List;

/**
 * StringListTypeHandler List类型数据库字段转换
 *
 * @author z00445440
 * @since 2023-01-09
 */
@MappedTypes({List.class})
public class StringListTypeHandler extends ListTypeHandler<String> {
    @Override
    protected TypeReference<List<String>> specificType() {
        return new TypeReference<List<String>>() {
        };
    }
}
