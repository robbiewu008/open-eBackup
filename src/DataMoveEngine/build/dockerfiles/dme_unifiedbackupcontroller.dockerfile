ARG VERSION_3RD
FROM dme_3rd:$VERSION_3RD as base_3rd

FROM open-ebackup-1.0-cbb-python:base
ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/lib64:/usr/lib:/usr/local/lib
WORKDIR /opt/OceanStor/100P/ProtectEngine-E/dme_unifiedbackupcontroller
COPY --chown=15025:99 /opt/OceanStor/100P/ProtectEngine-E/dme_unifiedbackupcontroller /opt/OceanStor/100P/ProtectEngine-E/dme_unifiedbackupcontroller
RUN useradd dme_unifiedbackupcontroller -M -N -s /sbin/nologin  -u 15025 -g 99 && \
    ldconfig && awk '{print $0}' /opt/.env app/common/config/.env >> app/.env && \
    rm -rf /opt/.env app/common/config/.env && \
    echo "dme_unifiedbackupcontroller   ALL=(ALL) NOPASSWD:/usr/bin/mount_oper.sh" >> /etc/sudoers.d/dme && \
    echo "dme_unifiedbackupcontroller   ALL=(ALL) NOPASSWD:/opt/FileClient/install/init_incluster.sh" >> /etc/sudoers.d/dme && \
    chmod 500 /usr/bin/mount_oper.sh && \
    pip3 install -i https://cmc.centralrepo.rnd.huawei.com/artifactory/pypi-central-repo/simple \
    --trusted-host cmc.centralrepo.rnd.huawei.com --no-cache-dir -r requirements.txt && \
    rm -rf requirements.txt && chown -R 15025:99 /opt/OceanStor && chown -R root:99 /usr/local/lib/python* && \
    rm -rf /usr/bin/restclient && rm -rf /usr/bin/kmcdecrypt && \
    chown -R root:99 /usr/local/lib64/python* && chmod 750 /opt/OceanStor/100P/ProtectEngine-E/dme_unifiedbackupcontroller \
    && rm -f /root/.config/pip/pip.conf \
    && sed -i 's/FileAdminServerPort=.*/FileAdminServerPort=30800/' /opt/FileClient/conf/hcpconf.ini \
    && sed -i 's/RandomPort=.*/RandomPort=0/' /opt/FileClient/conf/hcpconf.ini \
    && /opt/FileClient/install/install.sh  /opt/OceanProtect/protectmanager/cert/CA/certs/ca.crt.pem \
    /opt/OceanProtect/protectmanager/cert/internal/ProtectAgent/client.crt.pem  \
    /opt/OceanProtect/protectmanager/cert/internal/ProtectAgent/client.pem     \
    /opt/OceanProtect/protectmanager/kmc/master.ks /kmc_conf/..data/backup.ks  \
    /opt/OceanProtect/protectmanager/cert/internal/ProtectAgent/client.cnf
USER 15025
CMD ["python3","-m","app"]
