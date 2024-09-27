package openbackup.system.base.sdk.alarm.i18n;

/**
 * 国际化处理类
 *
 * @author l90005176
 * @version [LEGO V100R002C01, 2011-12-12]
 * @since 2018-01-01
 */
public class I18nMrgUtil {
    private static I18nMrgUtil i18nMrg;

    private I18nMrg i18nMgr = null;

    /**
     * 类实例
     *
     * @return I18nMrgUtil
     */
    public static synchronized I18nMrgUtil getInstance() {
        if (i18nMrg == null) {
            i18nMrg = new I18nMrgUtil();
        }
        return i18nMrg;
    }

    public I18nMrg getI18nMgr() {
        return i18nMgr;
    }

    public void setI18nMgr(I18nMrg i18nMgr) {
        this.i18nMgr = i18nMgr;
    }
}
