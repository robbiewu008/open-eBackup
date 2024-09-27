ARG VERSION_3RD
FROM dme_3rd:$VERSION_3RD as base_3rd

ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/lib64:/usr/lib:/usr/local/lib:/usr/local/lib64
WORKDIR /opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi_controller
COPY --chown=19726:99 /opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi_controller /opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi_controller
RUN useradd dme_openstorageapi_controller -M -N -s /sbin/nologin  -u 19726 -g 99 && \
    pip3 install -i https://cmc.centralrepo.rnd.huawei.com/artifactory/pypi-central-repo/simple \
    --trusted-host cmc.centralrepo.rnd.huawei.com --no-cache-dir -r requirements.txt && \
    rm -rf requirements.txt && chown -R 19726:99 /opt/OceanStor && chown -R root:99 /usr/local/lib/python* && \
    chown -R root:99 /usr/local/lib64/python* && \
    rm -rf /usr/bin/restclient && rm -rf /usr/bin/kmcdecrypt && \
    chmod 750 /opt/OceanStor/100P/ProtectEngine-E/dme_openstorageapi_controller \
    && rm -f /root/.config/pip/pip.conf
USER 19726
CMD ["python3","-m","app"]