FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake lcov libgmock-dev && \
    rm -rf /var/lib/apt/lists/* && \
    cd /usr/src/googletest && \
    cmake . && \
    make && \
    cp lib/*.a /usr/lib