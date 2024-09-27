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
FROM oceanprotect-dataprotect-1.0.rc1:base

WORKDIR /app
COPY MainServer.jar ./app.jar
COPY conf/                  ./conf/
COPY pkg/                   ./
COPY lib /usr/local/jre1.8.0_292/lib/aarch64/kmc

RUN chmod a+x /usr/local/jre1.8.0_292/lib/aarch64/kmc/* \
    && mv /usr/local/jre1.8.0_292/lib/aarch64/kmc/* /usr/local/jre1.8.0_292/lib/aarch64/ \
    && rm -rf /usr/local/jre1.8.0_292/lib/aarch64/kmc \
    && chmod -R +x bin/*.sh *.sh

CMD ["./app.sh"]
