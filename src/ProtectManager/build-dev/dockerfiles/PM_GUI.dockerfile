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
#FROM emei-il.huawei.com/misc/boilerplate/mvn:${mvn_tag} as build
FROM oceanprotect-dataprotect-1.0.rc1:base

WORKDIR /app
COPY gui.jar        ./app.jar

#RUN chmod 777 -R /app
#RUN chmod 777 -R /root
COPY app.sh                 .
COPY xml2json.py            .
RUN chmod +x ./app.sh
RUN luseradd -u 15012 -g nobody -s /sbin/nologin pm_gui

USER 15012
CMD ["./app.sh"]
