# stage 0: build binary
FROM sea5kg/ctf01d:build-environment-2026-05-20

COPY ./ /root/
WORKDIR /root/
RUN ./clean.sh && ./build_simple.sh

# WORKDIR /root/unit-tests.wsjcpp
# RUN ./build_simple.sh
# RUN ./unit-tests

# stage 1: release
FROM sea5kg/ctf01d:release-environment-2026-05-20
LABEL "maintainer"="Evgenii Sopov <mrseakg@gmail.com>"
LABEL "repository"="https://github.com/sea5kg/ctf01d"

RUN mkdir /root/data
COPY --from=0 /root/ctf01d /usr/bin/ctf01d

EXPOSE 8080
CMD ["ctf01d","start"]
