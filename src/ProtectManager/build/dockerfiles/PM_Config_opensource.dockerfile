ARG VERSION_3RD
FROM open-ebackup-1.0:base

ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/lib64:/usr/lib:/usr/local/lib:/usr/local/lib64
WORKDIR /opt/OceanStor/100P/ProtectManager/PM_Config
COPY --chown=99:99 /opt/OceanStor/100P/ProtectManager/PM_Config /opt/OceanStor/100P/ProtectManager/PM_Config
COPY --chown=99:99 /opt/OceanStor/100P/ProtectManager/3rd_inc /opt/OceanStor/100P/ProtectManager/3rd_inc
RUN yum install iproute -y
RUN luseradd -u 99 -g nobody -s /sbin/nologin nobody && \
    echo "nobody   ALL=(ALL) NOPASSWD:/usr/sbin/ip" >> /etc/sudoers && \
    pip3 install --no-index --find-links=../3rd_inc/ --no-cache-dir setuptools==65.6.3 && \
    pip3 install --no-index --find-links=../3rd_inc/ --no-cache-dir wheel==0.44.0 && \
    pip3 install --no-index --find-links=../3rd_inc/ --no-cache-dir -r requirements.txt && \
    rm -rf requirements.txt && chown -R 99:99 /opt/OceanStor && chown -R root:99 /usr/local/lib/python* && \
    chown -R root:99 /usr/local/lib64/python* && \
    rm -rf /usr/bin/restclient && rm -rf /usr/bin/kmcdecrypt && \
    chmod 750 /opt/OceanStor/100P/ProtectManager/PM_Config \
    && rm -f /root/.config/pip/pip.conf
USER 99
CMD ["python3","-m","app"]