# ---------------------------------------------------------------------
#
# $file: Dockerfile
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
COPY app.sh                 .
RUN mkdir -p /img/agent/client/ \
    && chmod +x ./app.sh
COPY DataProtect_${NewAppVersion}_client.zip      /img/agent/client/
COPY pm-crm-server-main.jar        ./app.jar

CMD ["./app.sh"]