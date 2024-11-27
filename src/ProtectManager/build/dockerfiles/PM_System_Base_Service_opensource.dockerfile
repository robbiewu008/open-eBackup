# ---------------------------------------------------------------------
#
# $file: Dockerfile
# $author: Protect Manager
#
# ---------------------------------------------------------------------

# ---------------------------------------------------------------------
# excute cmd to genarate jar file
# ---------------------------------------------------------------------
# mvn -Preal install -nsu -Dmaven.test.skip=true
# mvn -Pfake install -nsu -Dmaven.test.skip=true

#
# Build Service Image
#
#FROM ${docker_repo}/openjdk:${jdk_tag}
FROM open-ebackup-1.0:base

RUN luseradd -u 99 -g nobody -s /sbin/nologin nobody

WORKDIR /app
RUN chmod 750 /app \
    && chown -R 99:99 "/app"

COPY --chown=99:99 pm-main-server.jar ./app.jar
COPY --chown=99:99 conf/                  ./conf/
COPY --chown=99:99 pkg/                   ./
COPY --chown=99:99 lib /usr/local/${JDK_VERSION}/lib/aarch64/kmc

RUN mkdir -p /img/agent/client/ \
    && chmod 750 "/app/conf" \
    && chown 99:99 "/img" -R \
    && mkdir -p "/app/logs" \
    && chown 99:99 "/app/logs" \
    && chmod 750 "/app/logs" \
    && mkdir -p "/app/cert" \
    && chown 99:99 "/app/cert" \
    && chmod 750 "/app/cert" \
    && mkdir -p "/app/alarm" \
    && chown 99:99 "/app/alarm" \
    && chmod 750 "/app/alarm" \
    && mkdir -p "/app/agent" \
    && chown 99:99 "/app/agent" \
    && chmod 750 "/app/agent" \
    && mkdir -p "/script" \
    && chown root:nobody "/script" \
    && chmod 750 "/script" \
    && chmod a+x /usr/local/${JDK_VERSION}/lib/aarch64/kmc/* \
    && mv /usr/local/${JDK_VERSION}/lib/aarch64/kmc/* /usr/local/${JDK_VERSION}/lib/aarch64/ \
    && rm -rf /usr/local/${JDK_VERSION}/lib/aarch64/kmc \
    && chmod -R +x bin/*.sh *.sh \
    && touch "/etc/timezone" \
    && echo "nobody  ALL=(root)      NOPASSWD:/script/change_permission.sh * *" >> /etc/sudoers \
    && echo "nobody  ALL=(root)      NOPASSWD:/script/change_permission.sh * * *" >> /etc/sudoers \
    && echo "nobody  ALL=(root)      NOPASSWD:/script/curl_dorado_timezone.sh" >> /etc/sudoers \
    && echo "nobody  ALL=(root)      NOPASSWD:/script/mount_oper.sh  * * *" >> /etc/sudoers \
    && echo "nobody  ALL=(root)      NOPASSWD:/usr/sbin/route add *" >> /etc/sudoers \
    && echo "nobody  ALL=(root)      NOPASSWD:/usr/sbin/route del *" >> /etc/sudoers \
    && echo "nobody  ALL=(root)      NOPASSWD:/usr/bin/python3 /script/read_dorado_alarms_from_local.py *" >> /etc/sudoers \
    && echo "nobody  ALL=(root)      NOPASSWD:/usr/bin/python3 /script/update_hosts.py * *" >> /etc/sudoers \
    && echo "nobody  ALL=(root)      NOPASSWD:/usr/bin/python3 /script/delete_hosts.py *" >> /etc/sudoers \
    && echo "nobody  ALL=(root)      NOPASSWD:/script/sync_time.sh" >> /etc/sudoers \
    && echo "nobody  ALL=(root)      NOPASSWD:/script/change_dns.sh *" >> /etc/sudoers \
    && mv "/app/change_permission.sh" "/script" \
    && mv "/app/change_dns.sh" "/script" \
    && mv "/app/curl_dorado_timezone.sh" "/script" \
    && mv "/app/mount_oper.sh" "/script" \
    && mv "/app/sync_time.sh" "/script" \
    && mv "/app/read_dorado_alarms_from_local.py" "/script" \
    && mv "/app/update_hosts.py" "/script" \
    && mv "/app/delete_hosts.py" "/script" \
    && chmod 550 "/script/change_permission.sh" \
    && chmod 550 "/script/change_dns.sh" \
    && chmod 550 "/script/curl_dorado_timezone.sh" \
    && chmod 550 "/script/mount_oper.sh" \
    && chmod 550 "/script/sync_time.sh" \
    && chmod 550 "/script/read_dorado_alarms_from_local.py" \
    && chmod 550 "/script/update_hosts.py" \
    && chmod 550 "/script/delete_hosts.py" \
    && chown root:root "/script/change_permission.sh" \
    && chown root:root "/script/change_dns.sh" \
    && chown root:root "/script/curl_dorado_timezone.sh" \
    && chown root:root "/script/mount_oper.sh" \
    && chown root:root "/script/sync_time.sh" \
    && chown root:root "/script/read_dorado_alarms_from_local.py" \
    && chown root:root "/script/update_hosts.py" \
    && chown root:root "/script/delete_hosts.py"

RUN rpm -qa | grep ^libxcrypt-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^glibc-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^gcc-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-extra-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^cpp-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^make-[0-9] | xargs -i rpm -e {} --nodeps

RUN echo 'umask 0027' >> /etc/bashrc \
    && rm -rf /usr/bin/kmcdecrypt
COPY --chown=99:99 DataProtect_*_client.zip      /img/agent/client/

RUN chown nobody:nobody -R "/etc/timezone" \
    && chmod 640 "/app/conf/dpa.properties" \
    && rm -rf /root/.config/pip \
    && rm -rf /etc/localtime

CMD ["./app.sh"]
