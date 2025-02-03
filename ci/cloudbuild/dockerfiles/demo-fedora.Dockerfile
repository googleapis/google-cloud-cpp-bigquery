# Copyright 2024 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

FROM fedora:40
ARG NCPU=4

## [BEGIN packaging.md]

# Install the minimal development tools:

# ```bash
RUN dnf makecache && \
    dnf install -y cmake curl findutils gcc-c++ git make ninja-build \
        patch unzip tar wget zip
# ```

# Fedora:40 includes packages, with recent enough versions, for most of the
# direct dependencies of `google-cloud-cpp`.

# ```bash
RUN dnf makecache && \
    dnf install -y protobuf-compiler protobuf-devel grpc-cpp grpc-devel \
        json-devel libcurl-devel google-crc32c-devel openssl-devel
# ```

# #### Patching pkg-config

# If you are not planning to use `pkg-config(1)` you can skip these steps.

# Fedora's version of `pkg-config` (https://github.com/pkgconf/pkgconf) is slow
# when handling `.pc` files with lots of `Requires:` deps, which happens with
# Abseil. If you plan to use `pkg-config` with any of the installed artifacts,
# you may want to use a recent version of the standard `pkg-config` binary. If
# not, `dnf install pkgconfig` should work.

# ```bash
WORKDIR /var/tmp/build/pkgconf
RUN curl -fsSL https://distfiles.ariadne.space/pkgconf/pkgconf-2.2.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    ./configure --prefix=/usr --with-system-libdir=/lib64:/usr/lib64 --with-system-includedir=/usr/include && \
    make -j ${NCPU:-4} && \
    make install && \
    ldconfig && cd /var/tmp && rm -fr build
# ```

# Older versions of Fedora hard-code RE2 to use C++11. It was fixed starting
# with Fedora:38. If you using Fedora >= 38 or you are not planning to use
# `pkg-config(1)` you can ignore this step.  Alternatively, you can install RE2
# and gRPC from source.
# ```
# sed -i 's/-std=c\+\+11 //' /usr/lib64/pkgconfig/re2.pc
# ```

# The following steps will install libraries and tools in `/usr/local`. By
# default, pkgconf does not search in these directories. We need to explicitly
# set the search path.

# ```bash
ENV PKG_CONFIG_PATH=/usr/local/share/pkgconfig:/usr/lib64/pkgconfig:/usr/local/lib64/pkgconfig
# ```

# #### opentelemetry-cpp

# ```bash
WORKDIR /var/tmp/build/opentelemetry-cpp
RUN curl -fsSL https://github.com/open-telemetry/opentelemetry-cpp/archive/v1.19.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DWITH_EXAMPLES=OFF \
        -DWITH_ABSEIL=ON \
        -DBUILD_TESTING=OFF \
        -DOPENTELEMETRY_INSTALL=ON \
        -DOPENTELEMETRY_ABI_VERSION_NO=2 \
        -S . -B cmake-out && \
    cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
    ldconfig
# ```

# #### apache-arrow
WORKDIR /var/tmp/build/arrow
RUN curl -fsSL https://github.com/apache/arrow/archive/apache-arrow-18.1.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -GNinja -S cpp -B cmake-out \
      --preset ninja-release-minimal \
      -DARROW_JEMALLOC=OFF \
      -DBUILD_SHARED_LIBS=yes \
      -DARROW_BUILD_STATIC=ON  && \
    cmake --build cmake-out --target install
# ```

# #### google-cloud-cpp
WORKDIR /var/tmp/build/google-cloud-cpp
RUN curl -fsSL https://github.com/googleapis/google-cloud-cpp/archive/v2.35.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -GNinja -S . -B cmake-out \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE=bigquery,bigquerycontrol,opentelemetry && \
    cmake --build cmake-out --target install
# ```

## [DONE packaging.md]

WORKDIR /var/tmp/sccache
RUN curl -fsSL https://github.com/mozilla/sccache/releases/download/v0.8.2/sccache-v0.8.2-x86_64-unknown-linux-musl.tar.gz | \
    tar -zxf - --strip-components=1 && \
    mkdir -p /usr/local/bin && \
    mv sccache /usr/local/bin/sccache && \
    chmod +x /usr/local/bin/sccache

# Update the ld.conf cache in case any libraries were installed in /usr/local/lib*
RUN ldconfig /usr/local/lib*
