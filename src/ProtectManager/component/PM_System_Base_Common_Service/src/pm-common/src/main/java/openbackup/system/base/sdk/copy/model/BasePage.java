package openbackup.system.base.sdk.copy.model;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.livemount.model.Performance;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.BiFunction;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * Base Page
 *
 * @param <T> template type
 * @author l00272247
 * @since 2020-09-24
 */
@Data
public class BasePage<T> {
    private static final Logger LOGGER = LoggerFactory.getLogger(Performance.class);

    private long total;

    private long pages;

    @JsonProperty("page_size")
    private long pageSize;

    @JsonProperty("page_no")
    private long pageNo;

    private List<T> items;

    /**
     * query all
     *
     * @param function function
     * @param <T> template
     * @return page data
     */
    public static <T> BasePage<T> queryAll(BiFunction<Integer, Integer, BasePage<T>> function) {
        int size = IsmNumberConstant.TEN;
        boolean isRunning;
        AtomicInteger page = new AtomicInteger();
        List<T> itemsQueried = new ArrayList<>();
        do {
            BasePage<T> data = function.apply(page.get(), size);
            long totalQueried = page.getAndIncrement() * size + data.getItems().size();
            isRunning = totalQueried < data.getTotal();
            itemsQueried.addAll(data.getItems());
        } while (isRunning);
        return create(itemsQueried);
    }

    /**
     * create
     *
     * @param items items
     * @param <T> template
     * @return page data
     */
    public static <T> BasePage<T> create(List<T> items) {
        BasePage<T> data = new BasePage<>();
        data.setItems(items);
        data.setTotal(items.size());
        data.setPageSize(items.size());
        data.setPageNo(0);
        return data;
    }

    /**
     * get the first one
     *
     * @return the one item
     */
    public T one() {
        return !items.isEmpty() ? items.get(0) : null;
    }

    /**
     * get the one item
     *
     * @param exceptions exceptions
     * @return the one item
     */
    public T one(LegoCheckedException... exceptions) {
        if (exceptions != null && exceptions.length > 0) {
            if (total <= 0) {
                throw exceptions[0];
            } else if (total > IsmNumberConstant.ONE) {
                LegoCheckedException exception;
                if (exceptions.length > 1) {
                    exception = exceptions[1];
                } else {
                    exception = new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "Multiple resources found");
                }
                throw exception;
            } else {
                LOGGER.info("total error.");
            }
        }
        return one();
    }

    /**
     * map method
     *
     * @param function map function
     * @param <E> result template type
     * @return result page
     */
    public <E> BasePage<E> map(Function<T, E> function) {
        BasePage<E> page = new BasePage<>();
        page.setPages(getPages());
        page.setPageNo(getPageNo());
        page.setTotal(getTotal());
        page.setPageSize(getPageSize());
        page.setItems(getItems().stream().map(function).collect(Collectors.toList()));
        return page;
    }
}
