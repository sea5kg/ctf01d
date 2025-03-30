FROM python:latest
LABEL "maintainer"="Evgenii Sopoov <mrseakg@gmail.com>"
LABEL "repository"="https://github.com/sea-kg/ctf01d"

RUN apt-get -y update 
RUN apt-get install -y \
    python3 \
    python3-pip \
    && pip3 install requests

WORKDIR /root
COPY attack.py /root/attack.py

CMD python3 attack.py "127.0.0.1:8080" "team1" "127.0.0.1:4101,127.0.0.1:4201,127.0.0.1:4301"


