# ---------------------------------------------------------------------
#
# $file: Dockerfile
# $author: Protect Manager
#
# ---------------------------------------------------------------------

ARG jdk_tag=11.0.5-slim
ARG mvn_tag=3.6.3-1.3
ARG docker_repo=emei-il.huawei.com:5443

# ---------------------------------------------------------------------
# excute cmd to genarate jar file
# ---------------------------------------------------------------------
#mvn -Preal install -nsu -Dmaven.test.skip=true
#mvn -Pfake install -nsu -Dmaven.test.skip=true

#
#  Build Service Image
#
FROM ${docker_repo}/openjdk:${jdk_tag}
ARG jar_name=pm-main-server

WORKDIR /app
COPY ./src/Main/target/${jar_name}.jar         ./app.jar

COPY ./src/Main/src/main/resources/conf/       ./conf/
COPY ./src/KMS/encryp/KMC/kmc_source/release/lib          /usr/local/openjdk-11/lib

COPY scripts/app.sh .
RUN chmod +x ./app.sh

CMD ["./app.sh"]
