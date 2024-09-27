package openbackup.system.base.common.constants;

/**
 * Json Data View
 *
 * @author l00272247
 * @since 2019-11-28
 */
public class DataView {
    /**
     * 普通信息
     */
    public static class Normal {
        private Normal() {
        }
    }

    /**
     * 用于Email、电话号码等敏感信息
     */
    public static class Sensitive extends Normal {
        private Sensitive() {
        }
    }

    /**
     * 用于密码、Token等重要敏感信息
     */
    public static class Confidential extends Sensitive {
        private Confidential() {
        }
    }

    /**
     * 用于个人隐私等信息
     */
    public static class Personal extends Confidential {
        private Personal() {
        }
    }
}
