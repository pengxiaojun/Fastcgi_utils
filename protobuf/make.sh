#!/bin/sh

if [ -f protobuf/tvWall.pb.cpp ] && [ -f protobuf/tvWall.pb.h ]; then
    exit
fi

if [ ! -f protobuf/tvWall.proto ]; then
    exit
fi

protoc protobuf/tvWall.proto --cpp_out=.
mv protobuf/tvWall.pb.cc protobuf/tvWall.pb.cpp
