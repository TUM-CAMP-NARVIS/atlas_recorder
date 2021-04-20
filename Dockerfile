# syntax=docker/dockerfile:1.0.0-experimental
ARG from=registry.artekmed.inm.lan/artekmed_cn_base:v0.3-cuda11
FROM ${from}

WORKDIR /atlas_recorder

RUN --mount=type=ssh,id=github,required cd /atlas_recorder && \
    git clone -b master git@github.com:TUM-CAMP-NARVIS/atlas_recorder.git /atlas_recorder && \
    cd /pcp_workpace/ && \
    mkdir build && \
    cd build && \
    export CONAN_CPU_COUNT=$(nproc) && conan install .. --build "atlas_recorder" --build "missing" -s build_type=Release

WORKDIR /pcp_deploy/install

EXPOSE 55555 55556
EXPOSE 8889 8900 9876 9877 9878 9879 9880 9881
EXPOSE 31318
VOLUME ["/pcpd_config"]
VOLUME ["/pcpd_calib"]
VOLUME ["/pcpd_data"]

ENTRYPOINT ["/pcpd_config/pcpd_bootstrap.sh"]
CMD ["0.0.0.0:8889"]
