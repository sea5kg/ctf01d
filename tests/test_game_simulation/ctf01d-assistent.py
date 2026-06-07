#!/usr/bin/env python3
##################################################################################
#           Project
#   _______ _________ _______  _______  __    ______
#  (  ____ \\__   __/(  ____ \(  __   )/  \  (  __  \
#  | (    \/   ) (   | (    \/| (  )  |\/) ) | (  \  )
#  | |         | |   | (__    | | /   |  | | | |   ) |
#  | |         | |   |  __)   | (/ /) |  | | | |   | |
#  | |         | |   | (      |   / | |  | | | |   ) |
#  | (____/\   | |   | )      |  (__) |__) (_| (__/  )
#  (_______/   )_(   |/       (_______)\____/(______/
#
# MIT License
#
# Copyright (c) 2018-2026 Evgenii Sopov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Original repository: https://github.com/sea5kg/ctf01d
#
##################################################################################

import sys
import os
import time 
import datetime

simulation_dir = os.path.dirname(os.path.abspath(__file__))
project_root_dir = os.path.abspath(os.path.join(simulation_dir, "..", ".."))
os.chdir(simulation_dir)

try:
    import docker
except ModuleNotFoundError:
    print("Python module 'docker' is required. Install it with: python3 -m pip install docker")
    sys.exit(1)

# https://docker-py.readthedocs.io/en/latest/
client = docker.from_env()

progname = sys.argv[0]

# TODO read from data/config.yml

teams = [
    {
        "name": "team1",
        "ip_prefix": "10.10.11"
    }, {
        "name": "team2",
        "ip_prefix": "10.10.12"
    }, {
        "name": "team3",
        "ip_prefix": "10.10.13"
    }
]
jury_net_prefix = "10.10.100"
ntwrk_name_prefix = "ctf01d_net_"

services_list = ["service1_py", "service2_py", "service3_py", "service4_py"]
services_ports = {
    "service1_py": 4101,
    "service2_py": 4102,
    "service3_py": 4103,
    "service4_py": 4104,
}
img_name_prefix = "ctf01d-game-simulation/"

watchdog_containers_list = []

def printHelp():
    print("\n"
        + progname + " - assistent for manupulate game-simulation\n"
        "commands:\n"
        "\t clean - remove all containers, images, networks and etc...\n"
        "\t start - prepare networks, images, containers and etc \n"
        "\n"
    )

if len(sys.argv) < 2:
    printHelp()
    exit(1)

command = sys.argv[1]

def stopAndRemoveAllContainers():
    cntrs = client.containers.list(all=True)
    # print(cntrs)

    for c in cntrs:
        if c.name.startswith("ctf01d_"):
            print("Stopping container " + c.name)
            c.stop()
            print("Removing container " + c.name)
            c.remove()

def createNetwork(network_list, network_name, ip_prefix):
    for n in network_list:
        if n.name == network_name:
            print("Network already exists " + network_name)
            return 
    print("Creating network " + network_name)
    ipam_pool = docker.types.IPAMPool(
        subnet=ip_prefix + '.0/24',
        iprange=ip_prefix + '.0/24',
        gateway=ip_prefix + '.1',
        aux_addresses={
            'vuln_server': ip_prefix + '.2'
        }
    )
    ipam_config = docker.types.IPAMConfig(pool_configs=[ipam_pool])

    ret = client.networks.create(
        network_name,
        driver="bridge",
        check_duplicate=True,
        ipam=ipam_config
    )
    """
    simular command:
        $ docker network create \
        --driver bridge \
        --gateway 10.10.11.1 \
        --subnet=10.10.11.0/24 \
        --ip-range=10.10.11.3/24 \
        "ctf01d_net_team1"
    """
    print(" -> Done." + str(ret))

def createTeamAndJuryNetworks():
    print("\n ===> Create networks")
    ntwrks = client.networks.list()
    createNetwork(ntwrks, ntwrk_name_prefix + "jury", jury_net_prefix)
    for t in teams:
        # print(t)
        network_name_team = ntwrk_name_prefix + t['name']
        createNetwork(ntwrks, network_name_team, t['ip_prefix'])

def buildImage(images_list, image_tag, pathWithDockerfile):
    if not os.path.isdir(pathWithDockerfile):
        print(pathWithDockerfile + " - not exists")
        exit(-1)
        return

    path_dockerfile = pathWithDockerfile + "/Dockerfile"
    if not os.path.isfile(path_dockerfile):
        print(path_dockerfile + " - not exists")
        exit(-1)
        return

    for i in images_list:
        tag = ""
        if len(i.tags) > 0:
            tag = i.tags[0]
        if tag == image_tag:
            print(image_tag + " - Skip. Image already exists.")
            return
    # TODO redesing to command line build
    """
        simular command:
        docker build -t ctf01d-game-simulation/service1_py:latest ./vulnbox/service1_py/
    """
    print("Building image " + image_tag + "...")
    ret = client.images.build(
        path=pathWithDockerfile,
        tag=image_tag,
        forcerm=True # Always remove intermediate containers, even after unsuccessful builds
    )
    print(" -> Done." + str(ret))

def buildJuryAndServiceImages():
    print("\n ===> Docker images for Services")
    imgs = client.images.list()
    # basic image with jury
    buildImage(imgs, "sea5kg/ctf01d:latest", project_root_dir)
    for service_name in services_list:
        img_tag = img_name_prefix + service_name + ":latest"
        buildImage(imgs, img_tag, "./vulnbox/" + service_name)

def runAllPythonService(service_name, port):
    print(" ===> Starting all " + service_name)

    img_name = img_name_prefix + service_name + ":latest"
    for t in teams:
        dirname_flags = os.getcwd() + "/./tmp/" + t['name'] + "_" + service_name + "_flags"
        if not os.path.isdir(dirname_flags):
            os.mkdir(dirname_flags)
        network_name_team = ntwrk_name_prefix + t['name']
        container_name = "ctf01d_" + t['name'] + "_" + service_name
        cntrs = client.containers.list(all=True)
        for c in cntrs:
            if c.name == container_name:
                print("Stopping container " + c.name)
                c.stop()
                print("Removing container " + c.name)
                c.remove()

        print("Starting " + container_name)
        mount_flags = docker.types.Mount(
            target="/root/flags",
            source=dirname_flags,
            type="bind"
        )
        container = client.containers.run(
            img_name,
            mem_limit="128M",
            memswap_limit="128M",
            mounts=[mount_flags],
            network=network_name_team,
            ports={str(port) + "/tcp": (t['ip_prefix'] + ".1", port) },
            name=container_name,
            detach=True
        )
        """
        simular command:
          docker run \
            -d \
            --memory 128MB \
            --memory-swap 128MB \
            -p "10.10.11.1:4101":4101 \
            -v `pwd`/vulnbox/service1_py/flags:/root/flags \
            --net "ctf01d_net_team1" \
            --name "ctf01d_team1_service1_py" \
            ctf01d-game-simulation/service1_py:latest
        """
        watchdog_containers_list.append(container_name)
        print(container)

def runCtf01dJuryDb():
    print(" ===> Starting ctf01d-jury db")
    dirname_mysql = os.getcwd() + "/./tmp/jury_db_mysql"
    if not os.path.isdir(dirname_mysql):
        os.mkdir(dirname_mysql)
    network_name_team = ntwrk_name_prefix + "jury"
    container_name = "ctf01d_jury_db"
    cntrs = client.containers.list(all=True)
    for c in cntrs:
        if c.name == container_name:
            print("Stopping container " + c.name)
            c.stop()
            print("Removing container " + c.name)
            c.remove()

    print("Starting " + container_name)
    mount_mysql = docker.types.Mount(
        target="/var/lib/mysql",
        source=dirname_mysql,
        type="bind"
    )

    container = client.containers.run(
        "mysql:8",
        mem_limit="128M",
        memswap_limit="128M",
        mounts=[mount_mysql],
        environment={
            "MYSQL_ROOT_PASSWORD": "KzhyntJxwt",
            "MYSQL_DATABASE": "ctf01d",
            "MYSQL_USER": "ctf01d",
            "MYSQL_PASSWORD": "ctf01d",
        },
        network=network_name_team,
        # ports={"4101/tcp": (t['ip_prefix'] + ".1", 4101) },
        name=container_name,
        detach=True,
        command="mysqld --default-authentication-plugin=mysql_native_password"
    )
    """
    simular command:
        docker run -d \
            --name ctf01d-mysql \
            -e MYSQL_ROOT_PASSWORD=KzhyntJxwt \
            -e MYSQL_DATABASE=ctf01d \
            -e MYSQL_USER=ctf01d \
            -e MYSQL_PASSWORD=ctf01d \
            --network ctf01d_net \
            mysql:8 \
            mysqld --default-authentication-plugin=mysql_native_password
    """
    watchdog_containers_list.append(container_name)
    print(container)
    

def runCtf01dJury():
    print(" ===> Starting ctf01d-jury")
    
    dirname_data = os.getcwd() + "/data/"
    network_name_jury = ntwrk_name_prefix + "jury"
    container_name = "ctf01d_jury"
    cntrs = client.containers.list(all=True)
    for c in cntrs:
        if c.name == container_name:
            print("Stopping container " + c.name)
            c.stop()
            print("Removing container " + c.name)
            c.remove()

    print("Starting " + container_name)
    mount_data = docker.types.Mount(
        target="/root/data",
        source=dirname_data,
        type="bind"
    )
    container = client.containers.run(
        "sea5kg/ctf01d:latest",
        mem_limit="256M",
        memswap_limit="256M",
        mounts=[mount_data],
        network=network_name_jury,
        ports={"8080/tcp": ("127.0.0.1", 8080) },
        name=container_name,
        detach=True
    )
    watchdog_containers_list.append(container_name)
    print(container)

def startWatchDog():
    try:
        print(" ===> Starting watch dog")
        while True:
            cntrs = client.containers.list(all=True)
            for wc in watchdog_containers_list:
                for c in cntrs:
                    if c.name == wc and c.status == "exited":
                        today = datetime.datetime.today()
                        print(today.strftime("%Y%m%d-%H%M%S") + " Container " + wc + " is exited. Try start again")
                        c.start()
            time.sleep(15)
    except KeyboardInterrupt:
        print('Bye! Write me letters!')
        stopAndRemoveAllContainers()

if command == "stop":
    stopAndRemoveAllContainers()

if command == "clean":
    stopAndRemoveAllContainers()
    ntwrks = client.networks.list()
    for n in ntwrks:
        if n.name.startswith(ntwrk_name_prefix):
            print("Removing network " + n.name)
            n.remove()

    # find and remove images
    imgs = client.images.list()
    for i in imgs:
         tag = ""
         if len(i.tags) > 0:
             tag = i.tags[0]
         if tag.startswith(img_name_prefix):
             print("Removing image " + tag)
             client.images.remove(image=tag)
    exit(0)

if command == "start":
    if not os.path.isdir("./tmp"):
        os.mkdir("./tmp")
    createTeamAndJuryNetworks()
    buildJuryAndServiceImages()
    for service_name in services_list:
        runAllPythonService(service_name, services_ports[service_name])
    
    runCtf01dJury()
    startWatchDog()
