ARG VERSION_3RD
FROM dme_3rd:$VERSION_3RD
WORKDIR /opt/OceanStor/100P/ProtectEngine-E/dme_archive
RUN rm -f /usr/lib64/libkmcv3.so
COPY --chown=99:99 /opt/OceanStor/100P /opt/OceanStor/100P
RUN yum install -y nfs4-acl-tools && \
    echo -e "/opt/OceanStor/100P/ProtectEngine-E/dme_archive/lib\n/opt/OceanStor/100P/ProtectEngine-E/3rd\n/opt/OceanStor/100P/ProtectEngine-E/dme_archive/bin/services" >> /etc/ld.so.conf && \
    ldconfig && sh /opt/OceanStor/100P/ProtectEngine-E/install_ms.sh && \
    rm -rf /opt/OceanStor/100P/ProtectEngine-E/install_ms.sh && \
    mkdir -p /opt/OceanStor/root_exec && \
    mv -f /opt/OceanStor/100P/ProtectEngine-E/ms_mount.sh /opt/OceanStor/root_exec && \
    chmod -R 550 /opt/OceanStor/root_exec && chown -R root:root /opt/OceanStor/root_exec && \
    mkdir -p /opt/huawei-data-protection/vmware && \
    chmod 750 /opt/huawei-data-protection && \
    chmod 750 /opt/huawei-data-protection/vmware && \
    chown 99:99 /opt/huawei-data-protection && \
    chown 99:99 /opt/huawei-data-protection/vmware && \
    chown root:nobody /opt && chown root:nobody /opt/OceanStor && chown root:nobody /opt/OceanStor/100P && chown root:nobody /opt/OceanStor/100P/ProtectEngine-E && \
    chown root:nobody /opt/OceanStor/100P/ProtectEngine-E/dme_archive && chown root:nobody /opt/OceanStor/100P/ProtectEngine-E/dme_archive/bin && \
    chown root:nobody /opt/OceanStor/100P/ProtectEngine-E/dme_archive/script && chown root:nobody /opt/OceanStor/100P/ProtectEngine-E/dme_archive/script/dmeservice && \
    chown root:root /opt/OceanStor/100P/ProtectEngine-E/dme_archive/script/start.sh && chown root:root /opt/OceanStor/100P/ProtectEngine-E/dme_archive/script/dmeservice/check_health.sh && \
    chown root:root /opt/OceanStor/100P/ProtectEngine-E/dme_archive/bin/dme_archive &&\
    chmod 550 /opt/OceanStor/100P/ProtectEngine-E/dme_archive/script/*.sh && chmod 550 /opt/OceanStor/100P/ProtectEngine-E/dme_archive/script/utils/*.sh && \
    chmod 550 /opt/OceanStor/100P/ProtectEngine-E/dme_archive/script/dsware/*.sh && \
    setcap cap_dac_override+ep /opt/OceanStor/100P/ProtectEngine-E/dme_archive/bin/dme_archive &&\
    echo "Inside docker files:" && ls -laR /opt/OceanStor && rm -rf /usr/bin/kmcdecrypt \
    && rm -f /root/.config/pip/pip.conf

WORKDIR /opt/OceanStor/100P/ProtectEngine-E/3rd
RUN cp libpq.so.5.5 /usr/lib64/
CMD ["/bin/bash","-c","/opt/OceanStor/100P/ProtectEngine-E/dme_archive/script/start.sh"]
