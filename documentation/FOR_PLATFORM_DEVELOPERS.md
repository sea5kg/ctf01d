# For Platform Developers

## Ubuntu 20.04

Install package-requirements

```
sudo apt install git git-core \
    make cmake g++ pkg-config \
    libcurl4-openssl-dev \
    zlibc zlib1g zlib1g-dev \
    libpng-dev
```

Clone source code of the project:
```
$ git clone https://github.com/sea5kg/ctf01d ~/ctf01d.git
```

Build:
```
$ cd ~/ctf01d.git
$ ./build_simple.sh
```

Start:
```
$ cd ~/ctf01d.git
$ mkdir data_test
$ ./ctf01d -work-dir ./data_sample start
```

## Build in docker

1. In the first step we prepare docker network:

```
docker network create --driver=bridge ctf01d_net
```

We can look for docker status: `docker ps -a`

2. Prepare docker for builds:

*Notice: multistage build docker*

Or build fresh images for stages:
```
$ cd ~/ctf01d.git/contrib/docker-build-stages/
$ ./build-stages-images.sh
```

You can see them in a list:
```
$ docker images
```

And now you can build image:

```
$ cd ~/ctf01d.git
$ docker build --rm=true -t "sea5kg/ctf01d:latest" .
$ docker tag "sea5kg/ctf01d:latest" "sea5kg/ctf01d:v0.5.x"
```

3. Run dev docker-container, build and start

Run:
```
$ cd ~/ctf01d.git
$ docker run -it --rm \
  -p 8081:8080 \
  -v `pwd`/:/root/ctf01d.dev \
  -w /root/ctf01d.dev \
  --name "ctf01d.dev" \
  --network ctf01d_net \
  sea5kg/ctf01d:stage-build-latest \
  bash
root@604feda3c718:~/ctf01d.dev#
```

Build:
```
root@604feda3c718:~/ctf01d.dev# ./clean.sh
root@604feda3c718:~/ctf01d.dev# ./build_simple.sh
```

Start:
```
root@604feda3c718:~/ctf01d.dev# ./ctf01d -work-dir ./data_sample/ start
```

Now you can see scoreboard on http://localhost:8081

## Build release docker

```
docker build . -t sea5kg/ctf01d:v0.5.2
docker tag sea5kg/ctf01d:v0.5.2 sea5kg/ctf01d:latest
```