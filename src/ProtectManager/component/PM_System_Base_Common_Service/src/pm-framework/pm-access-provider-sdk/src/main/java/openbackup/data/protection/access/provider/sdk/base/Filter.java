package openbackup.data.protection.access.provider.sdk.base;

import com.fasterxml.jackson.annotation.JsonAlias;

import lombok.Data;

/**
 * Filter Entity
 *
 * @author l00272247
 * @since 2020-07-14
 */
@Data
public class Filter {
    // 1表示文件，2表示目录，3表示类型，4表示日期
    @JsonAlias("type")
    int filterType;

    // 1表示排除，2表示包含
    @JsonAlias("model")
    int filterMode;

    // 过滤内容
    Object content;
}
