# ---------------------------------------------------------------------
#
# $file: Dockerfile
# $author: Protect Manager
#
# ---------------------------------------------------------------------

ARG jdk_tag=11.0.5-slim
ARG mvn_tag=3.6.3-1.3
ARG docker_repo=emei-il.huawei.com:5443

FROM node as console
WORKDIR /app
COPY    src/service/console       .
RUN npm config set registry http://emei-il.huawei.com/nexus/repository/npm-proxy/
RUN npm i --unsafe-perm
RUN npm run build


# ---------------------------------------------------------------------
# Pull requirements maven packages
# ---------------------------------------------------------------------
FROM emei-il.huawei.com/misc/boilerplate/mvn:${mvn_tag} as build

ARG service_name=gui
WORKDIR /app
RUN mkdir -p build

COPY    src/service/src       ./src
COPY    src/service/pom.xml   .
COPY --from=console /app/dist/pm-gui ./src/main/resources/static

#
#
RUN echo =============  TEST FAKE ========================
RUN mvn -Pfake install  -nsu

RUN cp ./target/${service_name}-fake.jar    build/app-fake.jar

RUN mvn -Pfake clean

RUN echo =============  TEST REAL ========================
RUN mvn -Preal install  -nsu

RUN cp ./target/${service_name}.jar    build/app.jar

#
#  Build Service Image
#
FROM openjdk:${jdk_tag}

ARG JAR_PATH=/app/build


WORKDIR /app
COPY --from=build ${JAR_PATH}/*.jar ./

COPY scripts/app.sh .
COPY scripts/change_permission.sh .
COPY scripts/xml2json.py .
COPY scripts/read_dorado_alarms_from_local.py .
COPY scripts/check_health.sh .
COPY scripts/curl_dorado_timezone.sh .

RUN chmod +x ./app.sh
RUN chown -R 1001:1001 /app

CMD ["./app.sh"]
