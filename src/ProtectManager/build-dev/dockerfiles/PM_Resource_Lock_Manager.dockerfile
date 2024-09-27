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

CMD ["python3", "-m", "app"]
