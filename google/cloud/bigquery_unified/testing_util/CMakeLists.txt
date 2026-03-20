# ~~~
# Copyright 2024 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT BUILD_TESTING)
    return()
endif ()

add_library(
    google_cloud_cpp_bigquery_bigquery_unified_testing # cmake-format: sort
    opentelemetry_matchers.cc opentelemetry_matchers.h status_matchers.cc
    status_matchers.h)

if (GOOGLE_CLOUD_CPP_BIGQUERY_ENABLE_OPENTELEMETRY)
    find_package(google_cloud_cpp_opentelemetry CONFIG)
    if (TARGET google-cloud-cpp::opentelemetry)
        target_link_libraries(
            google_cloud_cpp_bigquery_bigquery_unified_testing
            PRIVATE opentelemetry-cpp::in_memory_span_exporter
                    opentelemetry-cpp::trace)
        target_compile_definitions(
            google_cloud_cpp_bigquery_bigquery_unified_testing
            PUBLIC # Enable OpenTelemetry features in google-cloud-cpp-bigquery
                   GOOGLE_CLOUD_CPP_BIGQUERY_HAVE_OPENTELEMETRY)
    endif ()
endif ()

target_include_directories(
    google_cloud_cpp_bigquery_bigquery_unified_testing
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
           $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
           $<INSTALL_INTERFACE:include>)

target_link_libraries(
    google_cloud_cpp_bigquery_bigquery_unified_testing
    PUBLIC absl::symbolize absl::failure_signal_handler
           google-cloud-cpp::common GTest::gmock)
google_cloud_cpp_add_common_options(
    google_cloud_cpp_bigquery_bigquery_unified_testing)
target_compile_definitions(
    google_cloud_cpp_bigquery_bigquery_unified_testing
    PRIVATE _GLIBCXX_USE_CXX11_ABI=1)

include(CheckCXXSymbolExists)
check_cxx_symbol_exists(getrusage sys/resource.h
                        GOOGLE_CLOUD_CPP_HAVE_GETRUSAGE)
check_cxx_symbol_exists(RUSAGE_THREAD sys/resource.h
                        GOOGLE_CLOUD_CPP_HAVE_RUSAGE_THREAD)

target_compile_definitions(
    google_cloud_cpp_bigquery_bigquery_unified_testing
    PUBLIC
        GOOGLE_CLOUD_CPP_HAVE_GETRUSAGE=$<BOOL:${GOOGLE_CLOUD_CPP_HAVE_GETRUSAGE}>
        GOOGLE_CLOUD_CPP_HAVE_RUSAGE_THREAD=$<BOOL:${GOOGLE_CLOUD_CPP_HAVE_RUSAGE_THREAD}>
)

create_bazel_config(google_cloud_cpp_bigquery_bigquery_unified_testing YEAR
                    2024)
