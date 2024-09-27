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
COPY conf/                  ./conf/

COPY pm-lm-server-main.jar        ./app.jar

COPY app.sh                 .
RUN chmod +x ./app.sh

CMD ["./app.sh"]
