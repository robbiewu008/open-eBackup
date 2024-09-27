ARG VERSION_3RD
FROM dme_3rd:$VERSION_3RD as base_3rd

ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/lib64:/usr/lib:/usr/local/lib:/usr/local/lib64
WORKDIR /opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi
COPY --chown=18245:99 /opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi /opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi
COPY --chown=18245:99 /opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi/app/common/script/mount_oper.sh /usr/bin/mount_oper.sh
RUN useradd dme_openstorageapi -M -N -s /sbin/nologin  -u 18245 -g 99 && \
    echo "dme_openstorageapi   ALL=(ALL) NOPASSWD:/usr/sbin/ip" >> /etc/sudoers && \
    echo "dme_openstorageapi   ALL=(ALL) NOPASSWD:/usr/bin/mount_oper.sh" >> /etc/sudoers && \
    chmod 500 /usr/bin/mount_oper.sh && \
    pip3 install -i https://cmc.centralrepo.rnd.huawei.com/artifactory/pypi-central-repo/simple \
    --trusted-host cmc.centralrepo.rnd.huawei.com --no-cache-dir -r requirements.txt && \
    rm -rf requirements.txt && chown -R 18245:99 /opt/OceanStor && chown -R root:99 /usr/local/lib/python* && \
    chown -R root:99 /usr/local/lib64/python* && \
    rm -rf /usr/bin/restclient && rm -rf /usr/bin/kmcdecrypt && \
    chmod 750 /opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi \
    && rm -f /root/.config/pip/pip.conf
USER 18245
CMD ["python3","-m","app"]