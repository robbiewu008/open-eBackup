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
FROM oceanprotect-dataprotect-1.0.rc1:base

WORKDIR /usr/local/lib
COPY package/3rd/libpq.so.5.5    .
RUN ln -s libpq.so.5.5 libpq.so.5 && ln -s libpq.so.5 libpq.so
COPY package/3rd/librdkafka_user_local/    /usr/local/
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib"
RUN ldconfig

# Build required packages
WORKDIR /wheels

# Add appoint packages
COPY package/3rd/ /wheels/

RUN sh prepare_env.sh dev && export LC_ALL=C && rm -rf /wheels/

# Copy app
WORKDIR /context/src
COPY package/src/  .
