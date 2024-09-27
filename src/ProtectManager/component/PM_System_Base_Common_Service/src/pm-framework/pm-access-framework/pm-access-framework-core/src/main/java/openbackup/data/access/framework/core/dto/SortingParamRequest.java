package openbackup.data.access.framework.core.dto;

import lombok.Data;

/**
 * Sorting parameter object
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2019-12-23
 */
@Data
public class SortingParamRequest {
    /**
     * 升序
     */
    public static final String ASC = "asc";

    /**
     * 降序
     */
    public static final String DESC = "desc";

    /**
     * 开始时间时间
     */
    public static final String START_TIME = "START_TIME";

    private String orderBy;

    private String orderType;
}
