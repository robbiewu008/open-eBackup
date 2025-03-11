ARG VERSION_3RD
FROM dme_3rd:$VERSION_3RD
WORKDIR /opt/OceanStor/100P/ProtectEngine-E/nginx/script
COPY --chown=65500:99 / /
RUN yum install iproute -y
RUN yum install ethtool -y
RUN useradd OceanProtectNginx -M -N -s /sbin/nologin  -u 65500 -g 99 && \
    python3 -m py_compile kmc_util.py && \
    cp -rf __pycache__/kmc_util.*.pyc kmc_util.pyc && \
    chown 65500:99 kmc_util.pyc && \
    chmod 750 kmc_util.pyc && \
    rm -rf __pycache__ kmc_util.py && \
    chown root:nobody nginx_mount.sh && \
    chown root:nobody /opt && chown root:nobody /opt/OceanStor && chown root:nobody /opt/OceanStor/100P && chown root:nobody /opt/OceanStor/100P/ProtectEngine-E && \
    chown root:nobody /opt/OceanStor/100P/ProtectEngine-E/nginx && chown root:nobody /opt/OceanStor/100P/ProtectEngine-E/nginx/script && \
    echo "OceanProtectNginx   ALL=(ALL) NOPASSWD:/opt/OceanStor/100P/ProtectEngine-E/nginx/script/nginx_mount.sh * *" >> /etc/sudoers.d/dme && \
    echo "OceanProtectNginx   ALL=(ALL) NOPASSWD:/usr/bin/grep ManageEthName /opt/product_conf" >> /etc/sudoers.d/dme && \
    sh /opt/OceanStor/100P/ProtectEngine-E/nginx/script/install_nginx.sh && \
    rm -rf /opt/OceanStor/100P/ProtectEngine-E/nginx/script/install_nginx.sh && \
    echo "Inside docker files:" && ls -laR /opt/OceanStor \
    && rm -f /root/.config/pip/pip.conf
CMD ["/bin/bash","-c","/opt/OceanStor/100P/ProtectEngine-E/nginx/script/start.sh"]