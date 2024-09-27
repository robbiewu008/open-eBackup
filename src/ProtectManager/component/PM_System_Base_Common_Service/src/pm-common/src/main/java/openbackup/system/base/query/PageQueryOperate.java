package openbackup.system.base.query;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;

import java.util.List;

/**
 * Page Query Operate
 *
 * @author l00272247
 * @since 2021-11-26
 */
public interface PageQueryOperate {
    /**
     * apply method
     *
     * @param wrapper wrapper
     * @param column column
     * @param parameters parameters
     * @param <T> template type
     * @return wrapper
     */
    <T> QueryWrapper<T> apply(QueryWrapper<T> wrapper, String column, List<?> parameters);
}
