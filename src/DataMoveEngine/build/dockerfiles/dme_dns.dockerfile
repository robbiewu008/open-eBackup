ARG VERSION_3RD
FROM dme_3rd:$VERSION_3RD
WORKDIR /opt/OceanStor/100P/ProtectEngine-E/dns/script
COPY --chown=99:99 /opt/OceanStor/100P /opt/OceanStor/100P
RUN chown -R root:root /opt/OceanStor/100P/ProtectEngine-E/dns && chmod -R 500 /opt/OceanStor/100P/ProtectEngine-E/dns && \
    echo "Inside docker files:" && ls -laR /opt/OceanStor && rm -rf /usr/bin/restclient && rm -rf /usr/bin/kmcdecrypt \
    && sed -i 's/user=dnsmasq/user=nobody/g' /etc/dnsmasq.conf && sed -i 's/group=dnsmasq/group=nobody/g' /etc/dnsmasq.conf \
    && rm -f /root/.config/pip/pip.conf
CMD ["/bin/bash","-c","/opt/OceanStor/100P/ProtectEngine-E/dns/script/start.sh"]
