ARG VERSION_3RD
FROM dme_3rd:$VERSION_3RD
WORKDIR /opt/OceanStor/100P/ProtectEngine-E/dme_replication
RUN luseradd nobody -M -s /sbin/nologin  -u 99 -g 99 && rm -f /usr/lib64/libkmcv3.so
COPY --chown=99:99 /opt/OceanStor/100P /opt/OceanStor/100P
RUN echo "nobody   ALL=(ALL) NOPASSWD:/opt/OceanStor/root_exec/ms_mount.sh" >> /etc/sudoers.d/dme && \
    echo "nobody ALL=(ALL) NOPASSWD:/opt/OceanStor/root_exec/nfs_oper.sh" >> /etc/sudoers.d/dme && \
    echo -e "/opt/OceanStor/100P/ProtectEngine-E/dme_replication/lib\n/opt/OceanStor/100P/ProtectEngine-E/3rd\n/opt/OceanStor/100P/ProtectEngine-E/dme_replication/bin/services" >> /etc/ld.so.conf && \
    ldconfig && sh /opt/OceanStor/100P/ProtectEngine-E/install_ms.sh && \
    setcap cap_dac_override+ep /opt/OceanStor/100P/ProtectEngine-E/dme_replication/bin/dme_replication && \
    rm -rf /opt/OceanStor/100P/ProtectEngine-E/install_ms.sh && \
    mkdir -p /opt/OceanStor/root_exec && \
    mv -f /opt/OceanStor/100P/ProtectEngine-E/ms_mount.sh /opt/OceanStor/root_exec && \
    mv -f /opt/OceanStor/100P/ProtectEngine-E/dme_replication/script/nfs_oper.sh /opt/OceanStor/root_exec && \
    mv -f /opt/OceanStor/100P/ProtectEngine-E/dme_replication/script/utils/ipTablesTool.sh /opt/OceanStor/root_exec && \
    mv -f /opt/OceanStor/100P/ProtectEngine-E/dme_replication/script/utils/fileOper.sh /opt/OceanStor/root_exec && \
    chmod -R 550 /opt/OceanStor/root_exec && chown -R root:root /opt/OceanStor/root_exec && \
    mkdir -p /opt/huawei-data-protection/metadata && \
    mkdir -p /mnt/replication && \
    chmod 750 /opt/huawei-data-protection && \
    chmod 750 /opt/huawei-data-protection/metadata && \
    chmod -R 750 /mnt/replication && \
    chown 99:99 /opt/huawei-data-protection && \
    chown 99:99 /opt/huawei-data-protection/metadata && \
    chown -R root:nobody /mnt/replication && \
    echo "Inside docker files:" && ls -laR /opt/OceanStor && rm -rf /usr/bin/kmcdecrypt && \
    echo "nobody ALL=(root) NOPASSWD: /opt/OceanStor/root_exec/ipTablesTool.sh" >>  /etc/sudoers && \
    echo "nobody ALL=(root) NOPASSWD: /opt/OceanStor/root_exec/fileOper.sh" >>  /etc/sudoers && \
    touch /run/xtables.lock \
    && rm -f /root/.config/pip/pip.conf \
    && echo "nobody  ALL=(ALL) NOPASSWD:/opt/FileClient/install/init_incluster.sh" >> /etc/sudoers.d/dme \
    && /opt/FileClient/install/install.sh  /opt/logpath/protectmanager/cert/CA/certs/ca.crt.pem \
    /opt/logpath/protectmanager/cert/internal/ProtectAgent/client.crt.pem \
    /opt/logpath/protectmanager/cert/internal/ProtectAgent/client.pem \
    /opt/logpath/protectmanager/kmc/master.ks /kmc_conf/..data/backup.ks \
    /opt/logpath/protectmanager/cert/internal/ProtectAgent/client.cnf

WORKDIR /opt/OceanStor/100P/ProtectEngine-E/3rd
RUN cp libpq.so.5.5 /usr/lib64/
USER 99
CMD ["/bin/bash","-c","/opt/OceanStor/100P/ProtectEngine-E/dme_replication/script/start.sh"]
