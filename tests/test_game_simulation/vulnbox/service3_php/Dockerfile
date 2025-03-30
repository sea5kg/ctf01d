FROM php:7.4-cli
LABEL "maintainer"="Evgenii Sopov <mrseakg@gmail.com>"
LABEL "repository"="https://github.com/sea-kg/ctf01d"

COPY ./src /usr/src/service3_php
WORKDIR /usr/src/service3_php
EXPOSE 4103

CMD [ "php", "-S", "0.0.0.0:4103"]

