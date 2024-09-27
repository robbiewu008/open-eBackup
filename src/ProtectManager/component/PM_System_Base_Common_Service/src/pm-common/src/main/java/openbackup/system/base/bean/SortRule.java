package openbackup.system.base.bean;

import lombok.Data;

import java.util.ArrayList;
import java.util.List;

/**
 * 排序规则
 *
 * @author w00607005
 * @since 2023-07-21
 */
@Data
public class SortRule {
    /**
     * 排序的字段
     */
    private String field;

    /**
     * 顺序
     */
    private List<?> order = new ArrayList<>();

    /**
     * 是否倒序
     */
    private boolean isReversed = false;

    public SortRule(String field, List<?> order, boolean isReversed) {
        this.field = field;
        this.order = order;
        this.isReversed = isReversed;
    }

    public SortRule(String field) {
        this(field, new ArrayList<>(), false);
    }

    public SortRule(String field, boolean isReversed) {
        this(field, new ArrayList<>(), isReversed);
    }

    public SortRule(String field, List<?> order) {
        this(field, order, false);
    }
}
