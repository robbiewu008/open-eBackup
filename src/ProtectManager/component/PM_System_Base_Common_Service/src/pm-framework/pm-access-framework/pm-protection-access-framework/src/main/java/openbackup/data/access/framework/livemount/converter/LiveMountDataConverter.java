package openbackup.data.access.framework.livemount.converter;

import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;
import openbackup.system.base.common.aspect.DataConverter;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collection;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Live Mount DataConverter
 *
 * @author l00272247
 * @since 2021-01-14
 */
@Component
public class LiveMountDataConverter implements DataConverter {
    @Autowired
    private LiveMountEntityDao liveMountEntityDao;

    /**
     * converter name
     *
     * @return converter name
     */
    @Override
    public String getName() {
        return "live_mount";
    }

    /**
     * convert data
     *
     * @param data data
     * @return result
     */
    @Override
    public Collection<?> convert(Collection<?> data) {
        List<String> list =
                data.stream().map(item -> item != null ? item.toString() : null).collect(Collectors.toList());
        return liveMountEntityDao.selectBatchIds(list);
    }
}
