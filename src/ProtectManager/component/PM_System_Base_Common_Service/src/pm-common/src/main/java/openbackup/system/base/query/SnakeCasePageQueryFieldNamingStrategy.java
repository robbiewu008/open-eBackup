package openbackup.system.base.query;

import com.fasterxml.jackson.databind.PropertyNamingStrategy;

import org.springframework.stereotype.Component;

/**
 * SnakeCasePageQueryFieldNamingStrategy
 *
 * @author l00272247
 * @since 2021-06-03
 */
@Component(SnakeCasePageQueryFieldNamingStrategy.NAME)
public class SnakeCasePageQueryFieldNamingStrategy extends DefaultPageQueryFieldNamingStrategy {
    /**
     * snakeCasePageQueryFieldNamingStrategy
     */
    public static final String NAME = "snakeCasePageQueryFieldNamingStrategy";

    /**
     * get Property Naming Strategy
     *
     * @param type type
     * @return result
     */
    @Override
    protected PropertyNamingStrategy.PropertyNamingStrategyBase getPropertyNamingStrategy(Class<?> type) {
        PropertyNamingStrategy.PropertyNamingStrategyBase strategy = super.getPropertyNamingStrategy(type);
        if (strategy != null) {
            return strategy;
        }
        return new PropertyNamingStrategy.SnakeCaseStrategy();
    }
}
