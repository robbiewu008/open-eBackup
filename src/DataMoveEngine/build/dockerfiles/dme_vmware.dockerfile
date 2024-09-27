ARG VERSION_3RD
FROM dme_3rd:$VERSION_3RD
WORKDIR /opt/OceanStor/100P/ProtectEngine-E/dme_vmware
RUN luseradd nobody -M -s /sbin/nologin  -u 99 -g 99
COPY --chown=99:99 /opt/OceanStor/100P /opt/OceanStor/100P

RUN echo "nobody   ALL=(ALL) NOPASSWD:/opt/OceanStor/root_exec/ms_mount.sh" >> /etc/sudoers.d/dme && \
    echo "nobody ALL=(ALL) NOPASSWD:/opt/OceanStor/root_exec/nfs_oper.sh" >> /etc/sudoers.d/dme && \
    mkdir /opt/OceanStor/root_exec && mkdir /mnt/vmware && \
    mv /opt/OceanStor/100P/ProtectEngine-E/ms_mount.sh /opt/OceanStor/root_exec && \
    mv /opt/OceanStor/100P/ProtectEngine-E/dme_vmware/script/nfs_oper.sh /opt/OceanStor/root_exec && \
    chown -R root:root /opt/OceanStor/root_exec && chmod -R 550 /opt/OceanStor/root_exec && \
	echo "nobody ALL=(ALL) NOPASSWD:/opt/OceanStor/100P/ProtectEngine-E/dme_vmware/script/nfs_oper.sh" >> /etc/sudoers.d/dme && \
    echo "nobody   ALL=(ALL) NOPASSWD:/usr/sbin/ndisc6" >> /etc/sudoers.d/dme && \
    echo -e "/opt/OceanStor/100P/ProtectEngine-E/dme_vmware/lib\n/opt/OceanStor/100P/ProtectEngine-E/3rd\n/opt/OceanStor/100P/ProtectEngine-E/dme_vmware/bin/services" >> /etc/ld.so.conf && \
    ldconfig && sh /opt/OceanStor/100P/ProtectEngine-E/install_ms.sh && \
    rm -rf /opt/OceanStor/100P/ProtectEngine-E/install_ms.sh && \
    mkdir -p /opt/huawei-data-protection/vmware && \
	mkdir -p /mnt/repository && \
    chmod 750 /opt/huawei-data-protection && \
    chmod 750 /opt/huawei-data-protection/vmware && \
    chmod 770 /mnt/repository && chown 99:99 /mnt/repository && \
    chown 99:99 /opt/huawei-data-protection && \
    chown 99:99 /opt/huawei-data-protection/vmware && \
    chmod 550 /opt/OceanStor/100P/ProtectEngine-E/dme_vmware/script/*.sh && chmod 550 /opt/OceanStor/100P/ProtectEngine-E/dme_vmware/script/utils/*.sh && \
    chmod 550 /opt/OceanStor/100P/ProtectEngine-E/dme_vmware/script/dsware/*.sh && \
    chmod 550 /opt/OceanStor/100P/ProtectEngine-E/dme_vmware/script/dmeservice/*.sh && \
    echo "Inside docker files:" && ls -laR /opt/OceanStor && rm -rf /usr/bin/kmcdecrypt \
    && rm -f /root/.config/pip/pip.conf

WORKDIR /opt/OceanStor/100P/ProtectEngine-E/3rd
RUN cp libpq.so.5.5 /usr/lib64/
USER 99
CMD ["/bin/bash","-c","/opt/OceanStor/100P/ProtectEngine-E/dme_vmware/script/start.sh"]
