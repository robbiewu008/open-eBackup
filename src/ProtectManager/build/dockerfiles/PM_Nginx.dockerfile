FROM oceanprotect-dataprotect-1.0.rc1:base

WORKDIR /usr/local/nginx

RUN luseradd -u 65500 -g nobody -s /sbin/nologin pm_nginx

RUN chmod 750 /usr/local/nginx \
    && chown 65500:99 /usr/local/nginx

COPY --chown=65500:99 package/ /usr/local/nginx

RUN rpm -qa | grep ^libxcrypt-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^glibc-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^gcc-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-devel-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-extra-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^binutils-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^cpp-[0-9] | xargs -i rpm -e {} --nodeps \
    && rpm -qa | grep ^make-[0-9] | xargs -i rpm -e {} --nodeps

RUN echo 'umask 0027' >> /etc/bashrc \
    && rm -rf /usr/bin/restclient \
    && mkdir -m 750 "/script" \
    && chown root:nobody "/script" \
    && echo "pm_nginx  ALL=(root)      NOPASSWD:/script/mount_oper.sh * * *" >> /etc/sudoers \
    && echo "pm_nginx  ALL=(root)      NOPASSWD:/script/change_permission.sh * *" >> /etc/sudoers \
    && mv "/usr/local/nginx/script/mount_oper.sh" "/script" \
    && mv "/usr/local/nginx/script/change_permission.sh" "/script" \
    && chmod 550 "/script/mount_oper.sh" \
    && chmod 550 "/script/change_permission.sh" \
    && chown root:root "/script/mount_oper.sh" \
    && chown root:root "/script/change_permission.sh" \
    && chmod 550 "/usr/local/nginx/nginx" \
    && chmod 660 "/usr/local/nginx/conf/nginx.conf" \
    && rm -rf /root/.config/pip


CMD ["sh", "script/start.sh"]
