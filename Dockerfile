# syntax=docker/dockerfile:1.0.0-experimental
ARG from=registry.artekmed.lan/artekmed_cn_base:v0.3-cuda11
# ARG from=registry.mobile.artekmed.lan/artekmed_cn_base:v0.3-cuda11
FROM ${from}

RUN sudo apt install software-properties-common && \
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test && \
    sudo apt update && \
    sudo apt install -y gcc-9 g++-9

RUN conan profile update settings.compiler.version=9 default

WORKDIR /atlas_recorder

ARG BRANCH=master

# trick to invalidate the cache if git version changes
ADD https://api.github.com/repos/TUM-CAMP-NARVIS/atlas_recorder/git/refs/heads/$BRANCH version.json

RUN --mount=type=ssh,id=github,required cd / && \
    git clone -b $BRANCH https://github.com/TUM-CAMP-NARVIS/atlas_recorder.git /pcp_deploy && \
    mkdir /pcp_deploy/install && \
    cd /pcp_deploy/install && \
    export CONAN_CPU_COUNT=$(nproc) && \
    CC=/usr/bin/gcc-9 CXX=/usr/bin/g++-9 conan install .. --build "missing" -s build_type=Release && \
    conan remove "*/*@*/*" --builds --force

RUN cd /pcp_deploy/install && cmake .. -DCMAKE_C_COMPILER=gcc-9 -DCMAKE_CXX_COMPILER=g++-9 && cmake --build . -j $(nproc)

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
