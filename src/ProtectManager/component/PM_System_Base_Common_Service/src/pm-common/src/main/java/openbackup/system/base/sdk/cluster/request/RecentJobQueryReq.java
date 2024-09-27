package openbackup.system.base.sdk.cluster.request;

import lombok.Data;

import java.util.ArrayList;
import java.util.List;

/**
 * RecentJobQueryReq
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-12-21
 */
@Data
public class RecentJobQueryReq {
    private List<String> statusList = new ArrayList<>();

    private int startPage = 0;

    private int pageSize = 10;

    private String orderType = "desc";

    private String orderBy = "start_time";

    private boolean isVisible = true;

    private boolean isSystem = false;
}
