package openbackup.system.base.common.log;

/**
 * data loader
 *
 * @author l00272247
 * @since 2019-11-05
 */
public interface OperationContextLoader {
    /**
     * data load method
     *
     * @param config data load config
     * @param value  value
     * @return result
     */
    Object load(OperationContextConfig config, Object value);
}

