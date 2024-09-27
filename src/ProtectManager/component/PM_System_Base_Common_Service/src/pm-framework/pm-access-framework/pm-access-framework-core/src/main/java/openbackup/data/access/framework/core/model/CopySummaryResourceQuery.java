package openbackup.data.access.framework.core.model;

import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 副本资源查询参数
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-03-02
 */
@Getter
@Setter
@EqualsAndHashCode
public class CopySummaryResourceQuery {
    private int pageNo;
    private int pageSize;
    private List<String> orders;
    private CopySummaryResourceCondition condition;
}
