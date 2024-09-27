FROM open-ebackup-1.0:base
WORKDIR /opt/OceanStor/100P/ProtectEngine-E/initcontainer
COPY --chown=99:99 /opt/OceanStor/100P /opt/OceanStor/100P
RUN echo "Inside docker files:" && ls -laR /opt/OceanStor && rm -rf /usr/bin/restclient && rm -rf /usr/bin/kmcdecrypt && rm -f /root/.config/pip/pip.conf
CMD ["/bin/bash","-c","/opt/OceanStor/100P/ProtectEngine-E/initcontainer/init_container.sh"]
