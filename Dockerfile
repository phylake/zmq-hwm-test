FROM ubuntu:16.04

RUN apt-get update

# ZMQ
WORKDIR /zeromq
RUN apt-get install -y cmake g++ python curl
RUN curl -L https://github.com/zeromq/libzmq/releases/download/v4.2.1/zeromq-4.2.1.tar.gz | tar -xzv
WORKDIR /zeromq/zeromq-4.2.1
RUN ./configure && make -j4 && make install && ldconfig

# CZMQ
WORKDIR /czmq
RUN curl -L https://github.com/zeromq/czmq/releases/download/v4.0.2/czmq-4.0.2.tar.gz | tar -xzv
WORKDIR /czmq/czmq-4.0.2
RUN ./configure && make -j4 && make install && ldconfig

WORKDIR /host/build
