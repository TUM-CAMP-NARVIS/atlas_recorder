# syntax=docker/dockerfile:1.0.0-experimental
ARG from=registry.artekmed.lan/artekmed_cn_base:v0.3-cuda11
FROM ${from}

WORKDIR /atlas_recorder

RUN --mount=type=ssh,id=github,required cd / && \
    git clone -b master https://github.com/TUM-CAMP-NARVIS/atlas_recorder.git /pcp_deploy && \
    mkdir /pcp_deploy/install && \
    cd /pcp_deploy/install && \
    export CONAN_CPU_COUNT=$(nproc) && conan install .. --build "missing" -s build_type=Release && \
    conan remove "*/*@*/*" --builds --force

RUN cd /pcp_deploy/install && cmake .. && cmake --build .

WORKDIR /pcp_deploy/install

ENV LD_LIBRARY_PATH /pcp_deploy/install/lib:${LD_LIBRARY_PATH}

EXPOSE 55555 55556
EXPOSE 8889 8900 9876 9877 9878 9879 9880 9881
EXPOSE 31318
VOLUME ["/pcpd_config"]
VOLUME ["/pcpd_calib"]
VOLUME ["/pcpd_data"]

ENTRYPOINT ["/pcpd_config/pcpd_bootstrap.sh"]
CMD ["0.0.0.0:8889"]
