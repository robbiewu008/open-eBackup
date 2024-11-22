# ---------------------------------------------------------------------
#
# $file: Dockerfile
# $author: Protect Manager
#
# ---------------------------------------------------------------------

ARG mvn_tag=3.6.3-1.3

# ---------------------------------------------------------------------
# excute cmd to genarate jar file
# ---------------------------------------------------------------------
# mvn -Preal install -nsu -Dmaven.test.skip=true
# mvn -Pfake install -nsu -Dmaven.test.skip=true

#
# Build Service Image
#
FROM open-ebackup-1.0:base

RUN luseradd -u 15012 -g nobody -s /sbin/nologin pm_gui

WORKDIR /app/gui
RUN chmod -R 750 /app
COPY  --chown=15012:99 gui.jar        ./app.jar
COPY  --chown=15012:99 app.sh                 .
COPY  --chown=15012:99 change_permission.sh   .
COPY --chown=15012:99 mount_oper.sh          .
COPY --chown=15012:99 xml2json.py            .
COPY --chown=15012:99 read_dorado_alarms_from_local.py          .
COPY --chown=15012:99 curl_dorado_timezone.sh          .
COPY --chown=15012:99 check_health.sh          .
COPY --chown=15012:99 init_cluster_role.sh          .
COPY --chown=15012:99 frontend ./frontend
COPY --chown=15012:99 lib /usr/local/${JDK_VERSION}/lib/aarch64/

RUN chmod +x ./app.sh
RUN yum install dejavu-sans-fonts fontconfig -y
RUN rpm -qa | grep ^libxcrypt-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^glibc-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^gcc-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-extra-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^cpp-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^make-[0-9] | xargs -i rpm -e {} --nodeps

RUN touch "/etc/timezone" \
    && mkdir -p "/app/gui/logs" \
    && chown 99:99 "/app/gui/logs" \
    && chmod 750 "/app/gui/logs" \
    && mkdir -p "/app/gui/frontend/console/assets/whitebox" \
    && chown 15012:99 "/app/gui/frontend/console/assets/whitebox" \
    && chmod 750 "/app/gui/frontend/console/assets/whitebox" \
    && mkdir -p "/app/gui/frontend/console/assets/deploy" \
    && chown 15012:99 "/app/gui/frontend/console/assets/deploy" \
    && chmod 750 "/app/gui/frontend/console/assets/deploy" \
    && chown 15012:99 -R "/app" \
    && chown 15012:99 -R "${JAVA_HOME}/lib/aarch64/" \
    && echo 'umask 0027' >> /etc/bashrc \
    && rm -rf /usr/bin/kmcdecrypt /usr/bin/restclient \
    && chown 15012:99 -R "/etc/timezone" \
    && mkdir -m 750 "/script" \
    && chown root:nobody "/script" \
    && chmod a+x /usr/local/${JDK_VERSION}/lib/aarch64/* \
    && mkdir -p "/app/gui/conf/wcc" \
    && chown 15012:99 -R "/app/gui/conf" \
    && echo "pm_gui  ALL=(root)      NOPASSWD:/script/change_permission.sh * *" >> /etc/sudoers \
    && echo "pm_gui  ALL=(root)      NOPASSWD:/script/change_permission.sh * * *" >> /etc/sudoers \
    && echo "pm_gui  ALL=(root)      NOPASSWD:/script/curl_dorado_timezone.sh" >> /etc/sudoers \
    && echo "pm_gui  ALL=(root)      NOPASSWD:/usr/bin/python3 /script/read_dorado_alarms_from_local.py *" >> /etc/sudoers \
    && echo "pm_gui  ALL=(root)      NOPASSWD:/usr/bin/sha256sum /whitebox/oem_package.tgz, /opt/ProtectManager/whitebox/oem_package.tgz" >> /etc/sudoers \
    && echo "pm_gui  ALL=(root)      NOPASSWD:/usr/bin/mv -f /whitebox/oem_package.tgz /opt/ProtectManager/whitebox" >> /etc/sudoers \
    && echo "pm_gui  ALL=(root)      NOPASSWD:/usr/bin/python3 /script/xml2json.py *" >> /etc/sudoers \
    && echo "pm_gui  ALL=(root)      NOPASSWD:/script/mount_oper.sh  * * *" >> /etc/sudoers \
    && mv "/app/gui/change_permission.sh" "/script" \
    && mv "/app/gui/curl_dorado_timezone.sh" "/script" \
    && mv "/app/gui/mount_oper.sh" "/script" \
    && mv "/app/gui/read_dorado_alarms_from_local.py" "/script" \
    && mv "/app/gui/xml2json.py" "/script" \
    && chmod 550 "/script/change_permission.sh" \
    && chmod 500 "/script/curl_dorado_timezone.sh" \
    && chmod 550 "/script/mount_oper.sh" \
    && chown root:root "/script/change_permission.sh" \
    && chown root:root "/script/curl_dorado_timezone.sh" \
    && chown root:root "/script/mount_oper.sh" \
    && chown root:root "/script/read_dorado_alarms_from_local.py" \
    && chown root:root "/script/xml2json.py" \
    && rm -rf /root/.config/pip


USER 15012
CMD ["./app.sh"]
