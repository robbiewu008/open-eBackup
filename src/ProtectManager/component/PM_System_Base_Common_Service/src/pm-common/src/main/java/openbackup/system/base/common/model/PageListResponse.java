package openbackup.system.base.common.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 分页模板类
 *
 * @param <T> the body type
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-06-29
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class PageListResponse<T> {
    private int totalCount;

    private int startIndex;

    private int pageSize;

    private int totalPages;

    private List<T> records;

    public PageListResponse(int totalCount, List<T> records) {
        this.totalCount = totalCount;
        this.records = records;
    }
}
