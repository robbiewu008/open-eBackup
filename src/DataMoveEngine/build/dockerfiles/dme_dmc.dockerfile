ARG VERSION_3RD
FROM dme_3rd:$VERSION_3RD
WORKDIR /opt/OceanStor/100P/ProtectEngine-E/dme_dmc
COPY --chown=15021:99 /opt/OceanStor/100P /opt/OceanStor/100P
RUN useradd dme_dmc -M -N -s /sbin/nologin  -u 15021 -g 99 && \
    echo "dme_dmc   ALL=(ALL) NOPASSWD:/opt/OceanStor/root_exec/ms_mount.sh" >> /etc/sudoers.d/dme && \
    mkdir /opt/OceanStor/root_exec && \
    mv /opt/OceanStor/100P/ProtectEngine-E/ms_mount.sh /opt/OceanStor/root_exec && \
    chown -R root:root /opt/OceanStor/root_exec && chmod -R 550 /opt/OceanStor/root_exec && \
    echo -e "/opt/OceanStor/100P/ProtectEngine-E/dme_dmc/lib\n/opt/OceanStor/100P/ProtectEngine-E/3rd\n/opt/OceanStor/100P/ProtectEngine-E/dme_dmc/bin/services" >> /etc/ld.so.conf && \
    ldconfig && sh /opt/OceanStor/100P/ProtectEngine-E/install_ms.sh && \
    rm -rf /opt/OceanStor/100P/ProtectEngine-E/install_ms.sh && \
    chmod 550 /opt/OceanStor/100P/ProtectEngine-E/dme_dmc/script/*.sh && chmod 550 /opt/OceanStor/100P/ProtectEngine-E/dme_dmc/script/utils/*.sh && \
    chmod 550 /opt/OceanStor/100P/ProtectEngine-E/dme_dmc/script/dsware/*.sh && \
    chmod 550 /opt/OceanStor/100P/ProtectEngine-E/dme_dmc/script/dmeservice/*.sh && \
    echo "Inside docker files:" && ls -laR /opt/OceanStor && rm -rf /usr/bin/kmcdecrypt \
    && rm -f /root/.config/pip/pip.conf

WORKDIR /opt/OceanStor/100P/ProtectEngine-E/3rd
RUN cp libpq.so.5.5 /usr/lib64/
USER 15021
CMD ["/bin/bash","-c","/opt/OceanStor/100P/ProtectEngine-E/dme_dmc/script/start.sh"]
