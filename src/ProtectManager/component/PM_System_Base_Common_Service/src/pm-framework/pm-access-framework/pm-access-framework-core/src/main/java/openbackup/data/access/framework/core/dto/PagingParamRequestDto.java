package openbackup.data.access.framework.core.dto;

import lombok.Data;

/**
 * Paging Param Request
 *
 * @author l00272247
 * @since 2020/07/03
 */
@Data
public class PagingParamRequestDto {
    private static final int DEFAULT_PAGE_SIZE = 10;

    private int startPage;

    private int pageSize;

    /**
     * constructor
     */
    public PagingParamRequestDto() {
        this(0, DEFAULT_PAGE_SIZE);
    }

    /**
     * constructor
     *
     * @param index index
     * @param count count
     */
    public PagingParamRequestDto(int index, int count) {
        this.startPage = index;
        this.pageSize = count;
    }
}
