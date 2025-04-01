FROM python:latest
LABEL "maintainer"="Evgenii Sopoov <mrseakg@gmail.com>"
LABEL "repository"="https://github.com/sea-kg/ctf01d"

WORKDIR /root
RUN mkdir ./flags && chmod 777 /root/flags
COPY service.py /root/service.py

EXPOSE 4101

CMD cd /root/ && python3 service.py


