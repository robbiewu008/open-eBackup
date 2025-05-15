# ---------------------------------------------------------------------
#
# $file: Dockerfile
# $author: Protect Manager
#
# ---------------------------------------------------------------------

ARG python_version=3.8-0.1
#ARG min_coverage=90

# ---------------------------------------------------------------------
# Pull requirements python packages
# ---------------------------------------------------------------------
FROM emei-il.huawei.com/misc/boilerplate/python-buster:${python_version} as dev_layer

# Build required packages
WORKDIR /wheels
COPY src/app/requirements requirements.app
RUN cat requirements.app > requirements
RUN pip wheel -r requirements

# Also install those packages
RUN pip install --no-index --find-links=/wheels -r requirements


# Add our top code folder to python path.
ENV PYTHONPATH=/context/src

# Copy everything
WORKDIR /context
COPY . .

CMD ["python", "-m", "app"]


## ---------------------------------------------------------------------
## Unit tests REAL+FAKE
## ---------------------------------------------------------------------
## Run unit tests on real and fake services. pytest also aggregated code coverage
## and reporting missing covergaes code lines
#FROM dev_layer as unit_test
#WORKDIR /context/tests/unit
#
## Run unit tests on both real and fake
#RUN python3 -m pytest --mode real --cov-config .coveragerc --cov=/context/src/app --cov-report=html && \
#    python3 -m pytest --mode fake --cov-config .coveragerc --cov=/context/src/app --cov-report=html --cov-append
#
## Expose coverage data
#WORKDIR /context/tests/unit/htmlcov
#CMD ["python", "-m", "http.server", "--cgi"]
#
#
## ---------------------------------------------------------------------
## Validations - Code formatting, Tags, and Coverage
## ---------------------------------------------------------------------
#FROM unit_test
#ARG min_coverage
#WORKDIR /context
#
## Check format
#RUN black --check -S --config service.toml .
#
## Check no placeholder left from boilerplate
#RUN echo python ./check-boilerplate-tags.py
#
#WORKDIR /context/tests/unit
#RUN python3 -m coverage report --fail-under=${min_coverage}


# ---------------------------------------------------------------------
# Build image
# ---------------------------------------------------------------------
FROM emei-il.huawei.com/misc/boilerplate/python-slim:${python_version} as build


WORKDIR /wheels
COPY --from=dev_layer /wheels .
RUN pip install --no-index --find-links=/wheels -r requirements.app && \
    rm -rf /wheels /root/.cache/pip/*

# copy the application source code only.
WORKDIR /src
COPY src/ .
ENV PYTHONPATH=/src IN_MS=Yes

CMD ["python", "-m", "app"]
