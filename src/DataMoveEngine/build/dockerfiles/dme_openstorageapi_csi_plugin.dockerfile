FROM open-ebackup-1.0:base
ARG binary
COPY ${binary} /app
ENTRYPOINT [ "/app" ]