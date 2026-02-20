# Packaging `google-cloud-cpp-bigquery`

This document is intended for:

- Package maintainers that need to understand how to compile `google-cloud-cpp-bigquery`
  from source to create their packaging scripts.
- Developers who may not be able to use a package manager and want to compile
  the `google-cloud-cpp-bigquery` libraries from source, including their dependencies.
- Developers who might like to install the `google-cloud-cpp-bigquery` libraries in
  `/usr/local` or a similar directory.

This document does not cover all cases for building `google-cloud-cpp-bigquery`. The
following are out of scope:

- Developers who prefer using a package manager such as
  [vcpkg](https://vcpkg.io), or [Conda](https://conda.io), should follow the
  instructions for their package manager.
- Developers wanting to use the libraries as part of a larger CMake or Bazel
  project should consult the [quickstart guides](/README.md#quickstart) for the
  library or libraries they want to use.
- Developers wanting to compile the library just to run examples or tests should
  consult the [building and installing](/README.md#building-and-installing)
  section of the top-level README file.
- Contributors and developers to `google-cloud-cpp-bigquery` should consult the guide to
  [set up a development workstation][howto-setup-dev-workstation].

## Getting the source code

There are two primary ways of obtaining `google-cloud-cpp-bigquery`'s source code. You
can use git:

```bash
git clone https://github.com/googleapis/google-cloud-cpp-bigquery.git $HOME/google-cloud-cpp-bigquery
```

Or obtain the tarball release (see
https://github.com/googleapis/google-cloud-cpp-bigquery/releases for the latest release):

```bash
VERSION="vX.Y.Z"
mkdir -p $HOME/google-cloud-cpp-bigquery
wget -q https://github.com/googleapis/google-cloud-cpp-bigquery/archive/${VERSION}.tar.gz
tar -xf ${VERSION}.tar.gz -C $HOME/google-cloud-cpp-bigquery --strip=1
```

## Installing `google-cloud-cpp-bigquery` with pre-existing dependencies

**If** all the dependencies of `google-cloud-cpp-bigquery` are installed and provide
CMake support files, then compiling and installing the libraries requires two
commands:

```bash
cmake -S . -B cmake-out -DBUILD_TESTING=OFF -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF
cmake --build cmake-out --target install
```

Unfortunately getting your system to this state may require multiple steps, the
following sections describe how to install `google-cloud-cpp-bigquery` on several
platforms.

## Common Configuration Variables for CMake

As is often the case, the CMake build can be configured using a number of
options and command-line flags. A full treatment of these options is outside the
scope of this document, but here are a few highlights:

- Consider using `-GNinja` to switch the generator from `make` (or msbuild on
  Windows) to \[`ninja`\][ninja-build]. In our experience `ninja` takes better
  advantage of multicore machines. Be aware that `ninja` is often not installed
  in development workstations, but it is available through most package
  managers.
- If you use the default generator, consider appending `-- -j ${NCPU}` to the
  build command, where `NCPU` is an environment variable set to the number of
  processors on your system. You can obtain this information using the `nproc`
  command on Linux, or `sysctl -n hw.physicalcpu` on macOS.
- By default, CMake compiles the `google-cloud-cpp-bigquery` as static libraries. The
  standard `-DBUILD_SHARED_LIBS=ON` option can be used to switch this to shared
  libraries. Having said this, on Windows there are [known issues][issues-5489]
  with DLLs and generated protos.

For more information about `google-cloud-cpp-bigquery` configuration options, see the
[Compile time configuration](/doc/compile-time-configuration.md) guide.

## Using `google-cloud-cpp-bigquery` after it is installed

Once installed, follow any of the [quickstart guides](/README.md#quickstart) to
use `google-cloud-cpp-bigquery` in your CMake or Make-based project. If you are planning
to use Bazel for your own project, there is no need to install
`google-cloud-cpp-bigquery`, we provide `BUILD.bazel` files for this purpose. The
quickstart guides also cover this use-case.

## Required Libraries

`google-cloud-cpp-bigquery` directly depends on the following libraries:

| Library                                 |   Minimum version | Description                                               |
| --------------------------------------- | ----------------: | --------------------------------------------------------- |
| [Abseil][abseil-gh]                     | 20240724, Patch 1 | Abseil C++ common library                                 |
| [gRPC][grpc-gh]                         |            1.69.x | An RPC library and framework                              |
| [libcurl][libcurl-gh]                   |            7.47.0 | HTTP client library                                       |
| [crc32c][crc32c-gh]                     |             1.0.6 | Hardware-accelerated CRC32C implementation                |
| [OpenSSL][openssl-gh]                   |             1.0.2 | Crypto functions                                          |
| [nlohmann/json][nlohmann-json-gh]       |             3.4.0 | JSON for Modern C++                                       |
| [protobuf][protobuf-gh]                 |            5.29.x | Protobuf is used for some RPC request and response types. |
| [google-cloud-cpp][google-cloud-cpp-gh] |            2.34.0 | GCP C++ Client libraries.                                 |
| [apache-arrow][apache-arrow-gh]         |            17.0.0 | BigQuery streams data using arrow::RecordBatch.           |
| [OpenTelemetry][opentelemetry-gh]       |             1.9.1 | An observability framework.                               |

Note that these libraries may also depend on other libraries. The following
instructions include steps to install these indirect dependencies too.

## Install Instructions per Platform

When possible, the instructions below prefer to use pre-packaged versions of
these libraries and their dependencies. In some cases the packages do not exist,
or the packaged versions are too old to support `google-cloud-cpp-bigquery`. If this is
the case, the instructions describe how you can manually download and install
these dependencies.

<!-- inject-distro-instructions-start -->

<details>
<summary>Alpine (Stable)</summary>
<br>

Install the minimal development tools, libcurl, and OpenSSL:

```bash
apk update && \
    apk add bash ca-certificates cmake curl git \
        gcc g++ make tar unzip zip zlib-dev
```

Alpine's version of `pkg-config` (https://github.com/pkgconf/pkgconf) is slow
when handling `.pc` files with lots of `Requires:` deps, which happens with
Abseil, so we use the normal `pkg-config` binary, which seems to not suffer
from this bottleneck. For more details see
https://github.com/pkgconf/pkgconf/issues/229 and
https://github.com/googleapis/google-cloud-cpp/issues/7052

```bash
mkdir -p $HOME/Downloads/pkgconf && cd $HOME/Downloads/pkgconf
curl -fsSL https://distfiles.ariadne.space/pkgconf/pkgconf-2.2.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    ./configure --prefix=/usr && \
    make -j ${NCPU:-4} && \
sudo make install && \
    cd /var/tmp && rm -fr build
```

The following steps will install libraries and tools in `/usr/local`. By
default, pkgconf does not search in these directories. We need to explicitly
set the search path.

```bash
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/lib/pkgconfig
```

#### Dependencies

The versions of Abseil, Protobuf, gRPC, OpenSSL, and nlohmann-json included
with Alpine >= 3.19 meet `google-cloud-cpp`'s requirements. We can simply
install the development packages

```bash
apk update && \
    apk add abseil-cpp-dev crc32c-dev c-ares-dev curl-dev grpc-dev \
        protobuf-dev ninja nlohmann-json openssl-dev re2-dev
```

#### opentelemetry-cpp

```bash
mkdir -p $HOME/Downloads/opentelemetry-cpp && cd $HOME/Downloads/opentelemetry-cpp
curl -fsSL https://github.com/open-telemetry/opentelemetry-cpp/archive/v1.19.0.tar.gz | \
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
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4}
```

#### apache-arrow

mkdir -p $HOME/Downloads/arrow && cd $HOME/Downloads/arrow
curl -fsSL https://github.com/apache/arrow/archive/apache-arrow-18.1.0.tar.gz | \
tar -xzf - --strip-components=1 && \
cmake \
-GNinja -S cpp -B cmake-out \
--preset ninja-release-minimal \
-DARROW_JEMALLOC=OFF \
-DBUILD_SHARED_LIBS=yes \
-DARROW_BUILD_STATIC=ON && \
sudo cmake --build cmake-out --target install

```

#### google-cloud-cpp
mkdir -p $HOME/Downloads/google-cloud-cpp && cd $HOME/Downloads/google-cloud-cpp
curl -fsSL https://github.com/googleapis/google-cloud-cpp/archive/v2.35.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -GNinja -S . -B cmake-out \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE=bigquery,bigquerycontrol,opentelemetry && \
sudo cmake --build cmake-out --target install
```

#### Compile and install the main project

We can now compile and install `google-cloud-cpp-bigquery`:

```bash
# Pick a location to install the artifacts, e.g., `/usr/local` or `/opt`
PREFIX="${HOME}/google-cloud-cpp-bigquery-installed"
cmake -S . -B cmake-out \
  -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
  -DBUILD_TESTING=OFF \
  -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE=__ga_libraries__,opentelemetry
cmake --build cmake-out -- -j "$(nproc)"
cmake --build cmake-out --target install
```

</details>

<details>
<summary>Fedora (40)</summary>
<br>

Install the minimal development tools:

```bash
sudo dnf makecache && \
sudo dnf install -y cmake curl findutils gcc-c++ git make ninja-build \
        patch unzip tar wget zip
```

Fedora:40 includes packages, with recent enough versions, for most of the
direct dependencies of `google-cloud-cpp`.

```bash
sudo dnf makecache && \
sudo dnf install -y protobuf-compiler protobuf-devel grpc-cpp grpc-devel \
        json-devel libcurl-devel google-crc32c-devel openssl-devel
```

#### Patching pkg-config

If you are not planning to use `pkg-config(1)` you can skip these steps.

Fedora's version of `pkg-config` (https://github.com/pkgconf/pkgconf) is slow
when handling `.pc` files with lots of `Requires:` deps, which happens with
Abseil. If you plan to use `pkg-config` with any of the installed artifacts,
you may want to use a recent version of the standard `pkg-config` binary. If
not, `sudo dnf install pkgconfig` should work.

```bash
mkdir -p $HOME/Downloads/pkgconf && cd $HOME/Downloads/pkgconf
curl -fsSL https://distfiles.ariadne.space/pkgconf/pkgconf-2.2.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    ./configure --prefix=/usr --with-system-libdir=/lib64:/usr/lib64 --with-system-includedir=/usr/include && \
    make -j ${NCPU:-4} && \
sudo make install && \
sudo ldconfig && cd /var/tmp && rm -fr build
```

Older versions of Fedora hard-code RE2 to use C++11. It was fixed starting
with Fedora:38. If you using Fedora >= 38 or you are not planning to use
`pkg-config(1)` you can ignore this step. Alternatively, you can install RE2
and gRPC from source.

```
sed -i 's/-std=c\+\+11 //' /usr/lib64/pkgconfig/re2.pc
```

The following steps will install libraries and tools in `/usr/local`. By
default, pkgconf does not search in these directories. We need to explicitly
set the search path.

```bash
export PKG_CONFIG_PATH=/usr/local/share/pkgconfig:/usr/lib64/pkgconfig:/usr/local/lib64/pkgconfig
```

#### opentelemetry-cpp

```bash
mkdir -p $HOME/Downloads/opentelemetry-cpp && cd $HOME/Downloads/opentelemetry-cpp
curl -fsSL https://github.com/open-telemetry/opentelemetry-cpp/archive/v1.19.0.tar.gz | \
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
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### apache-arrow

mkdir -p $HOME/Downloads/arrow && cd $HOME/Downloads/arrow
curl -fsSL https://github.com/apache/arrow/archive/apache-arrow-18.1.0.tar.gz | \
tar -xzf - --strip-components=1 && \
cmake \
-GNinja -S cpp -B cmake-out \
--preset ninja-release-minimal \
-DARROW_JEMALLOC=OFF \
-DBUILD_SHARED_LIBS=yes \
-DARROW_BUILD_STATIC=ON && \
sudo cmake --build cmake-out --target install

```

#### google-cloud-cpp
mkdir -p $HOME/Downloads/google-cloud-cpp && cd $HOME/Downloads/google-cloud-cpp
curl -fsSL https://github.com/googleapis/google-cloud-cpp/archive/v2.35.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -GNinja -S . -B cmake-out \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE=bigquery,bigquerycontrol,opentelemetry && \
sudo cmake --build cmake-out --target install
```

#### Compile and install the main project

We can now compile and install `google-cloud-cpp-bigquery`:

```bash
# Pick a location to install the artifacts, e.g., `/usr/local` or `/opt`
PREFIX="${HOME}/google-cloud-cpp-bigquery-installed"
cmake -S . -B cmake-out \
  -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
  -DBUILD_TESTING=OFF \
  -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE=__ga_libraries__,opentelemetry
cmake --build cmake-out -- -j "$(nproc)"
cmake --build cmake-out --target install
```

</details>

<details>
<summary>openSUSE (Leap)</summary>
<br>

Install the minimal development tools.

**NOTE:** The default compiler on openSUSE (GCC 7.5.0) crashes while compiling
some of the files generated by Protobuf. Minor variations in the Protobuf
version or the libraries changes where the compiler crashes. We recommend you
use GCC 8 or higher to compile `google-cloud-cpp`.

```bash
sudo zypper refresh && \
sudo zypper install --allow-downgrade -y automake cmake curl \
        gcc gcc-c++ gcc11 gcc11-c++ git gzip libtool make patch tar wget
```

Install some of the dependencies for `google-cloud-cpp`.

```bash
sudo zypper refresh && \
sudo zypper install --allow-downgrade -y abseil-cpp-devel c-ares-devel \
        libcurl-devel libopenssl-devel libcrc32c-devel nlohmann_json-devel \
        grpc-devel libprotobuf-devel ninja
```

The following steps will install libraries and tools in `/usr/local`. openSUSE
does not search for shared libraries in these directories by default. There
are multiple ways to solve this problem, the following steps are one solution:

```bash
(echo "/usr/local/lib" ; echo "/usr/local/lib64") | \
sudo tee /etc/ld.so.conf.d/usrlocal.conf
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig
export PATH=/usr/local/bin:${PATH}
```

Use the following environment variables to configure the compiler used by
CMake.

export CXX=g++-11
export CC=gcc-11

TODO(#64): Figure out what toolchain changes and additional dependencies that
need to be rebuilt in order to support C++17.

#### opentelemetry-cpp

```bash
mkdir -p $HOME/Downloads/opentelemetry-cpp && cd $HOME/Downloads/opentelemetry-cpp
curl -fsSL https://github.com/open-telemetry/opentelemetry-cpp/archive/v1.19.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DCMAKE_CXX_STANDARD=17 \
        -DWITH_EXAMPLES=OFF \
        -DWITH_ABSEIL=ON \
        -DBUILD_TESTING=OFF \
        -DOPENTELEMETRY_INSTALL=ON \
        -DOPENTELEMETRY_ABI_VERSION_NO=2 \
        -S . -B cmake-out && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### apache-arrow

mkdir -p $HOME/Downloads/arrow && cd $HOME/Downloads/arrow
curl -fsSL https://github.com/apache/arrow/archive/apache-arrow-18.1.0.tar.gz | \
tar -xzf - --strip-components=1 && \
cmake \
-GNinja -S cpp -B cmake-out \
--preset ninja-release-minimal \
-DCMAKE_CXX_STANDARD=17 \
-DARROW_JEMALLOC=OFF \
-DBUILD_SHARED_LIBS=yes \
-DARROW_BUILD_STATIC=ON && \
sudo cmake --build cmake-out --target install

```

#### google-cloud-cpp
mkdir -p $HOME/Downloads/google-cloud-cpp && cd $HOME/Downloads/google-cloud-cpp
curl -fsSL https://github.com/googleapis/google-cloud-cpp/archive/v2.35.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -GNinja -S . -B cmake-out \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_STANDARD=17 \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE=bigquery,bigquerycontrol,opentelemetry && \
sudo cmake --build cmake-out --target install
```

#### Compile and install the main project

We can now compile and install `google-cloud-cpp-bigquery`:

```bash
# Pick a location to install the artifacts, e.g., `/usr/local` or `/opt`
PREFIX="${HOME}/google-cloud-cpp-bigquery-installed"
cmake -S . -B cmake-out \
  -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
  -DBUILD_TESTING=OFF \
  -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE=__ga_libraries__,opentelemetry
cmake --build cmake-out -- -j "$(nproc)"
cmake --build cmake-out --target install
```

</details>

<details>
<summary>Ubuntu (24.04 LTS - Noble Numbat)</summary>
<br>

Install the minimal development tools, libcurl, OpenSSL and libc-ares:

```bash
export DEBIAN_FRONTEND=noninteractive
sudo apt-get update && \
sudo apt-get --no-install-recommends install -y apt-transport-https apt-utils \
        cmake ca-certificates curl git gcc g++ m4 make tar ninja-build
```

Ubuntu:24 includes packages for most of the direct dependencies of
`google-cloud-cpp`:

```bash
export DEBIAN_FRONTEND=noninteractive
sudo apt-get update && \
sudo apt-get --no-install-recommends install -y  \
        libabsl-dev \
        libcurl4-openssl-dev \
        libgrpc++-dev protobuf-compiler-grpc \
        libprotobuf-dev protobuf-compiler \
        nlohmann-json3-dev
```

#### Patching pkg-config

If you are not planning to use `pkg-config(1)` you can skip these steps.

Ubuntu's version of `pkg-config` (https://github.com/pkgconf/pkgconf) is slow
when handling `.pc` files with lots of `Requires:` deps, which happens with
Abseil. If you plan to use `pkg-config` with any of the installed artifacts,
you may want to use a recent version of the standard `pkg-config` binary. If
not, `sudo dnf install pkgconfig` should work.

```bash
mkdir -p $HOME/Downloads/pkgconf && cd $HOME/Downloads/pkgconf
rm -f /usr/bin/pkgconf /usr/bin/pkg-config
curl -fsSL https://distfiles.ariadne.space/pkgconf/pkgconf-2.2.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    ./configure --prefix=/usr -with-pkg-config-dir=/usr/local/lib/x86_64-linux-gnu/pkgconfig:/usr/local/lib/pkgconfig:/usr/local/share/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/lib/pkgconfig:/usr/share/pkgconfig && \
    make -j ${NCPU:-4} && \
sudo make install && \
sudo ldconfig && cd /var/tmp && rm -fr build
ln -s /usr/bin/pkgconf /usr/bin/pkg-config
```

#### crc32c

The project depends on the Crc32c library, we need to compile this from
source:

```bash
mkdir -p $HOME/Downloads/crc32c && cd $HOME/Downloads/crc32c
curl -fsSL https://github.com/google/crc32c/archive/1.1.2.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DCRC32C_BUILD_TESTS=OFF \
        -DCRC32C_BUILD_BENCHMARKS=OFF \
        -DCRC32C_USE_GLOG=OFF \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### opentelemetry-cpp

```bash
mkdir -p $HOME/Downloads/opentelemetry-cpp && cd $HOME/Downloads/opentelemetry-cpp
curl -fsSL https://github.com/open-telemetry/opentelemetry-cpp/archive/v1.19.0.tar.gz | \
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
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### apache-arrow

mkdir -p $HOME/Downloads/arrow && cd $HOME/Downloads/arrow
curl -fsSL https://github.com/apache/arrow/archive/apache-arrow-18.1.0.tar.gz | \
tar -xzf - --strip-components=1 && \
cmake \
-GNinja -S cpp -B cmake-out \
--preset ninja-release-minimal \
-DARROW_JEMALLOC=OFF \
-DBUILD_SHARED_LIBS=yes \
-DARROW_BUILD_STATIC=ON && \
sudo cmake --build cmake-out --target install

```

#### google-cloud-cpp
mkdir -p $HOME/Downloads/google-cloud-cpp && cd $HOME/Downloads/google-cloud-cpp
curl -fsSL https://github.com/googleapis/google-cloud-cpp/archive/v2.35.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -GNinja -S . -B cmake-out \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE=bigquery,bigquerycontrol,opentelemetry && \
sudo cmake --build cmake-out --target install
```

#### Compile and install the main project

We can now compile and install `google-cloud-cpp-bigquery`:

```bash
# Pick a location to install the artifacts, e.g., `/usr/local` or `/opt`
PREFIX="${HOME}/google-cloud-cpp-bigquery-installed"
cmake -S . -B cmake-out \
  -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
  -DBUILD_TESTING=OFF \
  -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE=__ga_libraries__,opentelemetry
cmake --build cmake-out -- -j "$(nproc)"
cmake --build cmake-out --target install
```

</details>

<details>
<summary>Ubuntu (22.04 LTS - Jammy Jellyfish)</summary>
<br>

Install the minimal development tools, libcurl, OpenSSL and libc-ares:

```bash
export DEBIAN_FRONTEND=noninteractive
sudo apt-get update && \
sudo apt-get --no-install-recommends install -y apt-transport-https apt-utils \
        automake build-essential cmake ca-certificates curl git \
        gcc g++ libc-ares-dev libc-ares2 libcurl4-openssl-dev libre2-dev \
        libssl-dev m4 make pkg-config tar wget zlib1g-dev ninja-build
```

#### Abseil

We need a recent version of Abseil. Enabling `ABSL_PROPAGATE_CXX_STD`
propagates the version of C++ used to compile Abseil to anything that depends
on Abseil.

```bash
mkdir -p $HOME/Downloads/abseil-cpp && cd $HOME/Downloads/abseil-cpp
curl -fsSL https://github.com/abseil/abseil-cpp/archive/20250127.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DABSL_BUILD_TESTING=OFF \
      -DABSL_PROPAGATE_CXX_STD=ON \
      -DBUILD_SHARED_LIBS=yes \
      -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### Protobuf

We need to install a version of Protobuf that is recent enough to support the
Google Cloud Platform proto files:

```bash
mkdir -p $HOME/Downloads/protobuf && cd $HOME/Downloads/protobuf
curl -fsSL https://github.com/protocolbuffers/protobuf/archive/v29.3.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_ABSL_PROVIDER=package \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### gRPC

We also need a version of gRPC that is recent enough to support the Google
Cloud Platform proto files. We install it using:

```bash
mkdir -p $HOME/Downloads/grpc && cd $HOME/Downloads/grpc
curl -fsSL https://github.com/grpc/grpc/archive/v1.78.1.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DgRPC_INSTALL=ON \
        -DgRPC_BUILD_TESTS=OFF \
        -DgRPC_ABSL_PROVIDER=package \
        -DgRPC_CARES_PROVIDER=package \
        -DgRPC_PROTOBUF_PROVIDER=package \
        -DgRPC_RE2_PROVIDER=package \
        -DgRPC_SSL_PROVIDER=package \
        -DgRPC_ZLIB_PROVIDER=package \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### crc32c

The project depends on the Crc32c library, we need to compile this from
source:

```bash
mkdir -p $HOME/Downloads/crc32c && cd $HOME/Downloads/crc32c
curl -fsSL https://github.com/google/crc32c/archive/1.1.2.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DCRC32C_BUILD_TESTS=OFF \
        -DCRC32C_BUILD_BENCHMARKS=OFF \
        -DCRC32C_USE_GLOG=OFF \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### nlohmann_json library

The project depends on the nlohmann_json library. We use CMake to
install it as this installs the necessary CMake configuration files.
Note that this is a header-only library, and often installed manually.
This leaves your environment without support for CMake pkg-config.

```bash
mkdir -p $HOME/Downloads/json && cd $HOME/Downloads/json
curl -fsSL https://github.com/nlohmann/json/archive/v3.11.3.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DJSON_BuildTests=OFF \
      -S . -B cmake-out && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### opentelemetry-cpp

```bash
mkdir -p $HOME/Downloads/opentelemetry-cpp && cd $HOME/Downloads/opentelemetry-cpp
curl -fsSL https://github.com/open-telemetry/opentelemetry-cpp/archive/v1.19.0.tar.gz | \
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
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### apache-arrow

mkdir -p $HOME/Downloads/arrow && cd $HOME/Downloads/arrow
curl -fsSL https://github.com/apache/arrow/archive/apache-arrow-18.1.0.tar.gz | \
tar -xzf - --strip-components=1 && \
cmake \
-GNinja -S cpp -B cmake-out \
--preset ninja-release-minimal \
-DARROW_JEMALLOC=OFF \
-DBUILD_SHARED_LIBS=yes \
-DARROW_BUILD_STATIC=ON && \
sudo cmake --build cmake-out --target install

```

#### google-cloud-cpp
mkdir -p $HOME/Downloads/google-cloud-cpp && cd $HOME/Downloads/google-cloud-cpp
curl -fsSL https://github.com/googleapis/google-cloud-cpp/archive/v2.35.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -GNinja -S . -B cmake-out \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE=bigquery,bigquerycontrol,opentelemetry && \
sudo cmake --build cmake-out --target install
```

#### Compile and install the main project

We can now compile and install `google-cloud-cpp-bigquery`:

```bash
# Pick a location to install the artifacts, e.g., `/usr/local` or `/opt`
PREFIX="${HOME}/google-cloud-cpp-bigquery-installed"
cmake -S . -B cmake-out \
  -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
  -DBUILD_TESTING=OFF \
  -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE=__ga_libraries__,opentelemetry
cmake --build cmake-out -- -j "$(nproc)"
cmake --build cmake-out --target install
```

</details>

<details>
<summary>Ubuntu (20.04 LTS - Focal Fossa)</summary>
<br>

Install the minimal development tools, libcurl, OpenSSL and libc-ares:

```bash
export DEBIAN_FRONTEND=noninteractive
sudo apt-get update && \
sudo apt-get --no-install-recommends install -y apt-transport-https apt-utils \
        automake build-essential cmake ca-certificates curl git \
        gcc g++ libc-ares-dev libc-ares2 libcurl4-openssl-dev \
        libssl-dev m4 make pkg-config tar wget zlib1g-dev ninja-build
```

#### Abseil

We need a recent version of Abseil. Enabling `ABSL_PROPAGATE_CXX_STD`
propagates the version of C++ used to compile Abseil to anything that depends
on Abseil.

```bash
mkdir -p $HOME/Downloads/abseil-cpp && cd $HOME/Downloads/abseil-cpp
curl -fsSL https://github.com/abseil/abseil-cpp/archive/20250127.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DABSL_BUILD_TESTING=OFF \
      -DABSL_PROPAGATE_CXX_STD=ON \
      -DBUILD_SHARED_LIBS=yes \
      -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### Protobuf

We need to install a version of Protobuf that is recent enough to support the
Google Cloud Platform proto files:

```bash
mkdir -p $HOME/Downloads/protobuf && cd $HOME/Downloads/protobuf
curl -fsSL https://github.com/protocolbuffers/protobuf/archive/v29.3.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_ABSL_PROVIDER=package \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### RE2

The version of RE2 included with this distro hard-codes C++11 in its
pkg-config file. You can skip this build and use the system's package if
you are not planning to use pkg-config.

```bash
mkdir -p $HOME/Downloads/re2 && cd $HOME/Downloads/re2
curl -fsSL https://github.com/google/re2/archive/2024-07-02.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DRE2_BUILD_TESTING=OFF \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### gRPC

We also need a version of gRPC that is recent enough to support the Google
Cloud Platform proto files. We install it using:

```bash
mkdir -p $HOME/Downloads/grpc && cd $HOME/Downloads/grpc
curl -fsSL https://github.com/grpc/grpc/archive/v1.78.1.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DgRPC_INSTALL=ON \
        -DgRPC_BUILD_TESTS=OFF \
        -DgRPC_ABSL_PROVIDER=package \
        -DgRPC_CARES_PROVIDER=package \
        -DgRPC_PROTOBUF_PROVIDER=package \
        -DgRPC_RE2_PROVIDER=package \
        -DgRPC_SSL_PROVIDER=package \
        -DgRPC_ZLIB_PROVIDER=package \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### crc32c

The project depends on the Crc32c library, we need to compile this from
source:

```bash
mkdir -p $HOME/Downloads/crc32c && cd $HOME/Downloads/crc32c
curl -fsSL https://github.com/google/crc32c/archive/1.1.2.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DCRC32C_BUILD_TESTS=OFF \
        -DCRC32C_BUILD_BENCHMARKS=OFF \
        -DCRC32C_USE_GLOG=OFF \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### nlohmann_json library

The project depends on the nlohmann_json library. We use CMake to
install it as this installs the necessary CMake configuration files.
Note that this is a header-only library, and often installed manually.
This leaves your environment without support for CMake pkg-config.

```bash
mkdir -p $HOME/Downloads/json && cd $HOME/Downloads/json
curl -fsSL https://github.com/nlohmann/json/archive/v3.11.3.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DJSON_BuildTests=OFF \
      -S . -B cmake-out && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### opentelemetry-cpp

```bash
mkdir -p $HOME/Downloads/opentelemetry-cpp && cd $HOME/Downloads/opentelemetry-cpp
curl -fsSL https://github.com/open-telemetry/opentelemetry-cpp/archive/v1.19.0.tar.gz | \
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
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### apache-arrow

mkdir -p $HOME/Downloads/arrow && cd $HOME/Downloads/arrow
curl -fsSL https://github.com/apache/arrow/archive/apache-arrow-18.1.0.tar.gz | \
tar -xzf - --strip-components=1 && \
cmake \
-GNinja -S cpp -B cmake-out \
--preset ninja-release-minimal \
-DCMAKE_BUILD_TYPE=Release \
-DARROW_MIMALLOC=OFF \
-DARROW_WITH_RE2=OFF \
-DARROW_WITH_UTF8PROC=OFF \
-DARROW_JEMALLOC=OFF \
-DBUILD_SHARED_LIBS=yes \
-DARROW_BUILD_STATIC=ON && \
sudo cmake --build cmake-out --target install

```

#### google-cloud-cpp
mkdir -p $HOME/Downloads/google-cloud-cpp && cd $HOME/Downloads/google-cloud-cpp
curl -fsSL https://github.com/googleapis/google-cloud-cpp/archive/v2.35.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -GNinja -S . -B cmake-out \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE=bigquery,bigquerycontrol,opentelemetry && \
sudo cmake --build cmake-out --target install
```

#### Compile and install the main project

We can now compile and install `google-cloud-cpp-bigquery`:

```bash
# Pick a location to install the artifacts, e.g., `/usr/local` or `/opt`
PREFIX="${HOME}/google-cloud-cpp-bigquery-installed"
cmake -S . -B cmake-out \
  -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
  -DBUILD_TESTING=OFF \
  -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE=__ga_libraries__,opentelemetry
cmake --build cmake-out -- -j "$(nproc)"
cmake --build cmake-out --target install
```

</details>

<details>
<summary>Debian (12 - Bookworm)</summary>
<br>

Install the minimal development tools.

```bash
sudo apt-get update && \
sudo apt-get --no-install-recommends install -y apt-transport-https apt-utils \
        automake build-essential ca-certificates cmake curl git \
        gcc g++ m4 make ninja-build pkg-config tar wget zlib1g-dev
```

Install the development packages for direct `google-cloud-cpp` dependencies:

```bash
sudo apt-get update && \
sudo apt-get --no-install-recommends install -y \
        libabsl-dev \
        libprotobuf-dev protobuf-compiler \
        libgrpc++-dev libgrpc-dev protobuf-compiler-grpc \
        libcurl4-openssl-dev libssl-dev nlohmann-json3-dev
```

#### Patching pkg-config

If you are not planning to use `pkg-config(1)` you can skip these steps.

Debian's version of `pkg-config` (https://github.com/pkgconf/pkgconf) is slow
when handling `.pc` files with lots of `Requires:` deps, which happens with
Abseil. If you plan to use `pkg-config` with any of the installed artifacts,
you may want to use a recent version of the standard `pkg-config` binary. If
not, `sudo dnf install pkgconfig` should work.

```bash
mkdir -p $HOME/Downloads/pkgconf && cd $HOME/Downloads/pkgconf
curl -fsSL https://distfiles.ariadne.space/pkgconf/pkgconf-2.2.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    ./configure --prefix=/usr --with-system-libdir=/lib:/usr/lib --with-system-includedir=/usr/include && \
    make -j ${NCPU:-4} && \
sudo make install && \
sudo ldconfig && cd /var/tmp && rm -fr build
export PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/local/lib/pkgconfig
```

#### crc32c

The project depends on the Crc32c library, we need to compile this from
source:

```bash
mkdir -p $HOME/Downloads/crc32c && cd $HOME/Downloads/crc32c
curl -fsSL https://github.com/google/crc32c/archive/1.1.2.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DCRC32C_BUILD_TESTS=OFF \
        -DCRC32C_BUILD_BENCHMARKS=OFF \
        -DCRC32C_USE_GLOG=OFF \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### opentelemetry-cpp

```bash
mkdir -p $HOME/Downloads/opentelemetry-cpp && cd $HOME/Downloads/opentelemetry-cpp
curl -fsSL https://github.com/open-telemetry/opentelemetry-cpp/archive/v1.19.0.tar.gz | \
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
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### apache-arrow

mkdir -p $HOME/Downloads/arrow && cd $HOME/Downloads/arrow
curl -fsSL https://github.com/apache/arrow/archive/apache-arrow-18.1.0.tar.gz | \
tar -xzf - --strip-components=1 && \
cmake \
-GNinja -S cpp -B cmake-out \
--preset ninja-release-minimal \
-DARROW_JEMALLOC=OFF \
-DBUILD_SHARED_LIBS=yes \
-DARROW_BUILD_STATIC=ON && \
sudo cmake --build cmake-out --target install

```

#### google-cloud-cpp
mkdir -p $HOME/Downloads/google-cloud-cpp && cd $HOME/Downloads/google-cloud-cpp
curl -fsSL https://github.com/googleapis/google-cloud-cpp/archive/v2.35.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -GNinja -S . -B cmake-out \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE=bigquery,bigquerycontrol,opentelemetry && \
sudo cmake --build cmake-out --target install
```

#### Compile and install the main project

We can now compile and install `google-cloud-cpp-bigquery`:

```bash
# Pick a location to install the artifacts, e.g., `/usr/local` or `/opt`
PREFIX="${HOME}/google-cloud-cpp-bigquery-installed"
cmake -S . -B cmake-out \
  -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
  -DBUILD_TESTING=OFF \
  -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE=__ga_libraries__,opentelemetry
cmake --build cmake-out -- -j "$(nproc)"
cmake --build cmake-out --target install
```

</details>

<details>
<summary>Debian (11 - Bullseye)</summary>
<br>

Install the minimal development tools, libcurl, and OpenSSL:

```bash
sudo apt-get update && \
sudo apt-get --no-install-recommends install -y apt-transport-https apt-utils \
        automake build-essential ca-certificates clang cmake curl git \
        gcc g++ libc-ares-dev libc-ares2 libcurl4-openssl-dev \
        libssl-dev m4 make ninja-build pkg-config tar wget zlib1g-dev
```

TODO(#64): Figure out what toolchain changes and additional dependencies that
need to be rebuilt in order to support C++17.

#### Abseil

Debian 11 ships with Abseil==20200923.3. Unfortunately, the current gRPC
version needs Abseil >= 20210324. Enabling `ABSL_PROPAGATE_CXX_STD`
propagates the version of C++ used to compile Abseil to anything that depends
on Abseil.

```bash
mkdir -p $HOME/Downloads/abseil-cpp && cd $HOME/Downloads/abseil-cpp
curl -fsSL https://github.com/abseil/abseil-cpp/archive/20250127.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_STANDARD=17 \
      -DABSL_CXX_STANDARD=17 \
      -DCMAKE_CXX_FLAGS='-D_GLIBCXX_USE_CXX11_ABI=0' \
      -DABSL_BUILD_TESTING=OFF \
      -DABSL_PROPAGATE_CXX_STD=ON \
      -DBUILD_SHARED_LIBS=yes \
      -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### crc32c

The project depends on the Crc32c library, we need to compile this from
source:

```bash
mkdir -p $HOME/Downloads/crc32c && cd $HOME/Downloads/crc32c
curl -fsSL https://github.com/google/crc32c/archive/1.1.2.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_CXX_FLAGS='-D_GLIBCXX_USE_CXX11_ABI=0' \
        -DBUILD_SHARED_LIBS=yes \
        -DCRC32C_BUILD_TESTS=OFF \
        -DCRC32C_BUILD_BENCHMARKS=OFF \
        -DCRC32C_USE_GLOG=OFF \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### nlohmann_json library

Debian 11 also ships with nlohmann-json==3.9.1, which is recent enough for our needs:

```bash
sudo apt-get update && \
sudo apt-get --no-install-recommends install -y nlohmann-json3-dev
```

#### Protobuf

Unless you are only using the Google Cloud Storage library the project
needs Protobuf and gRPC. Unfortunately the version of Protobuf that ships
with Debian 11 is not recent enough to support the protos published by
Google Cloud. We need to build from source:

```bash
mkdir -p $HOME/Downloads/protobuf && cd $HOME/Downloads/protobuf
curl -fsSL https://github.com/protocolbuffers/protobuf/archive/v29.3.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_CXX_FLAGS='-D_GLIBCXX_USE_CXX11_ABI=0' \
        -DBUILD_SHARED_LIBS=yes \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_ABSL_PROVIDER=package \
        -S . -B cmake-out && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### RE2

The version of RE2 included with this distro hard-codes C++11 in its
pkg-config file. You can skip this build and use the system's package if
you are not planning to use pkg-config.

```bash
mkdir -p $HOME/Downloads/re2 && cd $HOME/Downloads/re2
curl -fsSL https://github.com/google/re2/archive/2024-07-02.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_CXX_FLAGS='-D_GLIBCXX_USE_CXX11_ABI=0' \
        -DBUILD_SHARED_LIBS=ON \
        -DRE2_BUILD_TESTING=OFF \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### gRPC

Finally, we build gRPC from source:

```bash
mkdir -p $HOME/Downloads/grpc && cd $HOME/Downloads/grpc
curl -fsSL https://github.com/grpc/grpc/archive/v1.78.1.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_CXX_FLAGS='-D_GLIBCXX_USE_CXX11_ABI=0' \
        -DgRPC_INSTALL=ON \
        -DgRPC_BUILD_TESTS=OFF \
        -DgRPC_ABSL_PROVIDER=package \
        -DgRPC_CARES_PROVIDER=package \
        -DgRPC_PROTOBUF_PROVIDER=package \
        -DgRPC_RE2_PROVIDER=package \
        -DgRPC_SSL_PROVIDER=package \
        -DgRPC_ZLIB_PROVIDER=package \
        -S . -B cmake-out && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

```bash
mkdir -p $HOME/Downloads/opentelemetry-cpp && cd $HOME/Downloads/opentelemetry-cpp
curl -fsSL https://github.com/open-telemetry/opentelemetry-cpp/archive/v1.19.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_CXX_FLAGS='-D_GLIBCXX_USE_CXX11_ABI=0' \
        -DBUILD_SHARED_LIBS=yes \
        -DWITH_EXAMPLES=OFF \
        -DWITH_ABSEIL=ON \
        -DBUILD_TESTING=OFF \
        -DOPENTELEMETRY_INSTALL=ON \
        -DOPENTELEMETRY_ABI_VERSION_NO=2 \
        -S . -B cmake-out && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### apache-arrow

mkdir -p $HOME/Downloads/arrow && cd $HOME/Downloads/arrow
curl -fsSL https://github.com/apache/arrow/archive/apache-arrow-18.1.0.tar.gz | \
tar -xzf - --strip-components=1 && \
cmake \
-GNinja -S cpp -B cmake-out \
-DCMAKE_CXX_STANDARD=17 \
-DCMAKE_CXX_FLAGS='-D_GLIBCXX_USE_CXX11_ABI=0' \
-DARROW_JEMALLOC=OFF \
-DBUILD_SHARED_LIBS=yes \
-DARROW_BUILD_STATIC=ON && \
sudo cmake --build cmake-out --target install

```

#### google-cloud-cpp
mkdir -p $HOME/Downloads/google-cloud-cpp && cd $HOME/Downloads/google-cloud-cpp
curl -fsSL https://github.com/googleapis/google-cloud-cpp/archive/v2.35.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -GNinja -S . -B cmake-out \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_CXX_FLAGS='-D_GLIBCXX_USE_CXX11_ABI=0' \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE=bigquery,bigquerycontrol,opentelemetry && \
sudo cmake --build cmake-out --target install
```

#### Compile and install the main project

We can now compile and install `google-cloud-cpp-bigquery`:

```bash
# Pick a location to install the artifacts, e.g., `/usr/local` or `/opt`
PREFIX="${HOME}/google-cloud-cpp-bigquery-installed"
cmake -S . -B cmake-out \
  -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
  -DBUILD_TESTING=OFF \
  -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE=__ga_libraries__,opentelemetry
cmake --build cmake-out -- -j "$(nproc)"
cmake --build cmake-out --target install
```

</details>

<details>
<summary>Rocky Linux (9)</summary>
<br>

Install the minimal development tools, libcurl, OpenSSL, and the c-ares
library (required by gRPC):

```bash
sudo dnf makecache && \
sudo dnf update -y && \
sudo dnf install -y epel-release && \
sudo dnf makecache && \
sudo dnf install -y cmake findutils gcc-c++ git make openssl-devel \
        patch zlib-devel libcurl-devel c-ares-devel tar wget which

/usr/bin/crb enable && \
sudo dnf install -y ninja-build
```

Rocky Linux's version of `pkg-config` (https://github.com/pkgconf/pkgconf) is
slow when handling `.pc` files with lots of `Requires:` deps, which happens
with Abseil. If you plan to use `pkg-config` with any of the installed
artifacts, you may want to use a recent version of the standard `pkg-config`
binary. If not, `sudo dnf install pkgconfig` should work.

```bash
mkdir -p $HOME/Downloads/pkgconf && cd $HOME/Downloads/pkgconf
curl -fsSL https://distfiles.ariadne.space/pkgconf/pkgconf-2.2.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    ./configure --prefix=/usr --with-system-libdir=/lib64:/usr/lib64 --with-system-includedir=/usr/include && \
    make -j ${NCPU:-4} && \
sudo make install && \
sudo ldconfig && cd /var/tmp && rm -fr build
```

The following steps will install libraries and tools in `/usr/local`. By
default, Rocky Linux 9 does not search for shared libraries in these
directories, there are multiple ways to solve this problem, the following
steps are one solution:

```bash
(echo "/usr/local/lib" ; echo "/usr/local/lib64") | \
sudo tee /etc/ld.so.conf.d/usrlocal.conf
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:/usr/lib64/pkgconfig
export PATH=/usr/local/bin:${PATH}
```

#### Abseil

Rocky Linux 9 includes a package for Abseil, unfortunately, this package is
incomplete, as it lacks the CMake support files for it. We need to compile
Abseiil from source. Enabling `ABSL_PROPAGATE_CXX_STD` propagates the version
of C++ used to compile Abseil to anything that depends on Abseil.

```bash
mkdir -p $HOME/Downloads/abseil-cpp && cd $HOME/Downloads/abseil-cpp
curl -fsSL https://github.com/abseil/abseil-cpp/archive/20250127.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DABSL_BUILD_TESTING=OFF \
      -DABSL_PROPAGATE_CXX_STD=ON \
      -DBUILD_SHARED_LIBS=yes \
      -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### Protobuf

Rocky Linux ships with Protobuf 3.14.x. Some of the libraries in
`google-cloud-cpp` require Protobuf >= 3.15.8. For simplicity, we will just
install Protobuf (and any downstream packages) from source.

```bash
mkdir -p $HOME/Downloads/protobuf && cd $HOME/Downloads/protobuf
curl -fsSL https://github.com/protocolbuffers/protobuf/archive/v29.3.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_ABSL_PROVIDER=package \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### RE2

The version of RE2 included with this distro hard-codes C++11 in its
pkg-config file. You can skip this build and use the system's package if
you are not planning to use pkg-config.

```bash
mkdir -p $HOME/Downloads/re2 && cd $HOME/Downloads/re2
curl -fsSL https://github.com/google/re2/archive/2024-07-02.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DRE2_BUILD_TESTING=OFF \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### gRPC

We also need a version of gRPC that is recent enough to support the Google
Cloud Platform proto files. Note that gRPC overrides the default C++ standard
version to C++14, we need to configure it to use the platform's default. We
manually install it using:

```bash
mkdir -p $HOME/Downloads/grpc && cd $HOME/Downloads/grpc
curl -fsSL https://github.com/grpc/grpc/archive/v1.78.1.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_CXX_STANDARD=17 \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DgRPC_INSTALL=ON \
        -DgRPC_BUILD_TESTS=OFF \
        -DgRPC_ABSL_PROVIDER=package \
        -DgRPC_CARES_PROVIDER=package \
        -DgRPC_PROTOBUF_PROVIDER=package \
        -DgRPC_RE2_PROVIDER=package \
        -DgRPC_SSL_PROVIDER=package \
        -DgRPC_ZLIB_PROVIDER=package \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### crc32c

The project depends on the Crc32c library, we need to compile this from
source:

```bash
mkdir -p $HOME/Downloads/crc32c && cd $HOME/Downloads/crc32c
curl -fsSL https://github.com/google/crc32c/archive/1.1.2.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DCRC32C_BUILD_TESTS=OFF \
        -DCRC32C_BUILD_BENCHMARKS=OFF \
        -DCRC32C_USE_GLOG=OFF \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### nlohmann_json library

The project depends on the nlohmann_json library. We use CMake to
install it as this installs the necessary CMake configuration files.
Note that this is a header-only library, and often installed manually.
This leaves your environment without support for CMake pkg-config.

```bash
mkdir -p $HOME/Downloads/json && cd $HOME/Downloads/json
curl -fsSL https://github.com/nlohmann/json/archive/v3.11.3.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DJSON_BuildTests=OFF \
      -S . -B cmake-out && \
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### opentelemetry-cpp

```bash
mkdir -p $HOME/Downloads/opentelemetry-cpp && cd $HOME/Downloads/opentelemetry-cpp
curl -fsSL https://github.com/open-telemetry/opentelemetry-cpp/archive/v1.19.0.tar.gz | \
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
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
sudo ldconfig
```

#### apache-arrow

mkdir -p $HOME/Downloads/arrow && cd $HOME/Downloads/arrow
curl -fsSL https://github.com/apache/arrow/archive/apache-arrow-18.1.0.tar.gz | \
tar -xzf - --strip-components=1 && \
cmake \
-GNinja -S cpp -B cmake-out \
--preset ninja-release-minimal \
-DARROW_JEMALLOC=OFF \
-DBUILD_SHARED_LIBS=yes \
-DARROW_BUILD_STATIC=ON && \
sudo cmake --build cmake-out --target install

```

#### google-cloud-cpp
mkdir -p $HOME/Downloads/google-cloud-cpp && cd $HOME/Downloads/google-cloud-cpp
curl -fsSL https://github.com/googleapis/google-cloud-cpp/archive/v2.35.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -GNinja -S . -B cmake-out \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE=bigquery,bigquerycontrol,opentelemetry && \
sudo cmake --build cmake-out --target install
```

#### Compile and install the main project

We can now compile and install `google-cloud-cpp-bigquery`:

```bash
# Pick a location to install the artifacts, e.g., `/usr/local` or `/opt`
PREFIX="${HOME}/google-cloud-cpp-bigquery-installed"
cmake -S . -B cmake-out \
  -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
  -DBUILD_TESTING=OFF \
  -DGOOGLE_CLOUD_CPP_WITH_MOCKS=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE=__ga_libraries__,opentelemetry
cmake --build cmake-out -- -j "$(nproc)"
cmake --build cmake-out --target install
```

</details>
