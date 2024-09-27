package openbackup.system.base.sdk.kerberos;

/**
 * kerberos listener service
 *
 * @author c30016231
 * @since 2021-12-01
 */
public interface KerberosListener {
    /**
     * do something before delete
     *
     * @param kerberosId kerberosId
     */
    void beforeDelete(String kerberosId);
}
