package openbackup.system.base.query;

import java.util.List;

/**
 * Page Query Param
 *
 * @author l00272247
 * @since 2020-09-24
 */
public class PageQueryParam extends Pagination<String> {
    /**
     * constructor
     *
     * @param page       page
     * @param size       size
     * @param conditions conditions
     * @param orders     orders
     */
    public PageQueryParam(int page, int size, String conditions, List<String> orders) {
        super(page, size, conditions, orders);
    }
}
