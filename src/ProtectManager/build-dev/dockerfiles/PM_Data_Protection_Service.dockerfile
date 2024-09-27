# ---------------------------------------------------------------------
#
# $file: Dockerfile
# $author: Protect Manager
#
# ---------------------------------------------------------------------

# ---------------------------------------------------------------------
# Pull requirements python packages
# ---------------------------------------------------------------------

# FROM test:latest
FROM pm-app-common:VERSION

# Add our top code folder to python path.
ENV PYTHONPATH=/context/src

# Copy app
WORKDIR /context/src
COPY package/src/  .

RUN luseradd -u 15013 -g nobody -s /sbin/nologin pm_protection_service

USER 15013
CMD ["python3", "-m", "ptvsd", "--host", "0.0.0.0", "--port", "8190", "-m", "app"]
