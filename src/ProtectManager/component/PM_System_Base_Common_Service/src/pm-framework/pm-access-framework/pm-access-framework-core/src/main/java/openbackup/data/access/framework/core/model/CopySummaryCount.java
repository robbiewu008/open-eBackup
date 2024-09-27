package openbackup.data.access.framework.core.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 副本统计
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023/11/24
 */
@Getter
@Setter
public class CopySummaryCount {
    private String resourceSubType;
    private String resourceType;
    private int copyCount;
}
