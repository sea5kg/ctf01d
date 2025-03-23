#!/bin/bash


DT=$(date +"%Y%m%d")
TAG=$(echo "v$DT")
echo $TAG

docker rmi -f sea5kg/ctf01d:stage-build-$TAG
docker build -f Dockerfile.ctf01d-stage-build -t sea5kg/ctf01d:stage-build-$TAG .

docker rmi -f sea5kg/ctf01d:stage-release-$TAG
docker build -f Dockerfile.ctf01d-stage-release -t sea5kg/ctf01d:stage-release-$TAG .
