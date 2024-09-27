package openbackup.data.protection.access.provider.sdk.base;

import lombok.Data;

import java.util.List;

/**
 * Page List Response
 *
 * @param <T> template
 * @author l00272247
 * @since 2020-07-03
 */
@Data
public class PageListResponse<T> {
    private int totalCount;

    private List<T> records;

    /**
     * 构造函数
     *
     */
    public PageListResponse() {
    }

    /**
     * 构造函数
     *
     * @param totalCount 总数
     * @param records 结果集
     */
    public PageListResponse(int totalCount, List<T> records) {
        this.totalCount = totalCount;
        this.records = records;
    }
}
