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

include(FindGMockWithTargets)
include(GoogleCloudCppCommon)
include(CreateBazelConfig)

find_package(google_cloud_cpp_bigquery REQUIRED)
find_package(google_cloud_cpp_bigquerycontrol REQUIRED)
find_package(Arrow REQUIRED)

function (bigquery_unified_library library_name library_alias)
    cmake_parse_arguments(_opt "EXPERIMENTAL;TRANSITION;INSTALL" "INITIAL_YEAR"
                          "LIBRARY_FILES;DEPS;PKGCONFIG_DEPS" ${ARGN})

    add_library(${library_name} ${_opt_LIBRARY_FILES})

    target_include_directories(
        ${library_name}
        PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
               $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
               $<INSTALL_INTERFACE:include>)
    target_link_libraries(${library_name} PUBLIC ${_opt_DEPS})
    set_target_properties(
        ${library_name}
        PROPERTIES EXPORT_NAME "google-cloud-cpp-bigquery::${library_alias}"
                   VERSION "${PROJECT_VERSION}"
                   SOVERSION "${PROJECT_VERSION_MAJOR}")
    target_compile_options(${library_name}
                           PUBLIC ${GOOGLE_CLOUD_CPP_EXCEPTIONS_FLAG})
    add_library(google-cloud-cpp-bigquery::${library_alias} ALIAS
                ${library_name})

    create_bazel_config(${library_name} YEAR ${_opt_INITIAL_YEAR})

    if (${_opt_INSTALL})
        # Get the destination directories based on the GNU recommendations.
        include(GNUInstallDirs)

        # Export the CMake targets to make it easy to create configuration
        # files.
        install(
            EXPORT ${library_name}-targets
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${library_name}"
            COMPONENT google_cloud_cpp_bigquery_development)

        # Install the libraries and headers in the locations determined by
        # GNUInstallDirs
        install(
            TARGETS ${library_name}
            EXPORT ${library_name}-targets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                    COMPONENT google_cloud_cpp_bigquery_runtime
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                    COMPONENT google_cloud_cpp_bigquery_runtime
                    NAMELINK_COMPONENT google_cloud_cpp_bigquery_development
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                    COMPONENT google_cloud_cpp_bigquery_development)

        google_cloud_cpp_bigquery_install_headers(
            ${library_name} "include/google/cloud/bigquery_unified")

        google_cloud_cpp_bigquery_add_pkgconfig(
            ${library_alias} "The BigQuery Unified C++ Client Library"
            "Provides C++ APIs to use the BigQuery Unified API."
            ${_opt_PKGCONFIG_DEPS})

        # Create and install the CMake configuration files.
        include(CMakePackageConfigHelpers)
        set(GOOGLE_CLOUD_CPP_CONFIG_LIBRARY "${library_name}")

        foreach (lib IN LISTS _opt_PKGCONFIG_DEPS)
            list(APPEND find_dependencies "find_dependency(${lib})")
        endforeach ()

        string(JOIN "\n" GOOGLE_CLOUD_CPP_ADDITIONAL_FIND_DEPENDENCIES
               ${find_dependencies})
        configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/config.cmake.in"
                       "${library_name}-config.cmake" @ONLY)
        write_basic_package_version_file(
            "${library_name}-config-version.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY ExactVersion)

        install(
            FILES
                "${CMAKE_CURRENT_BINARY_DIR}/${library_name}-config.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/${library_name}-config-version.cmake"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${library_name}"
            COMPONENT google_cloud_cpp_bigquery_development)
    endif ()
endfunction ()

set(bigquery_unified_common_library_files
    # cmake-format: sort
    internal/version_info.h version.cc version.h)

set(bigquery_unified_common_library_deps # cmake-format: sort
                                         google-cloud-cpp::common)

set(bigquery_unified_common_library_pkgconfig_deps # cmake-format: sort
                                                   google_cloud_cpp_common)

bigquery_unified_library(
    google_cloud_cpp_bigquery_common
    common
    INSTALL
    INITIAL_YEAR
    "2024"
    LIBRARY_FILES
    ${bigquery_unified_common_library_files}
    DEPS
    ${bigquery_unified_common_library_deps}
    PKGCONFIG_DEPS
    ${bigquery_unified_common_library_pkgconfig_deps})

set(bigquery_unified_library_files
    # cmake-format: sort
    client.cc
    client.h
    connection.cc
    connection.h
    idempotency_policy.cc
    idempotency_policy.h
    internal/arrow_reader.cc
    internal/arrow_reader.h
    internal/async_rest_long_running_operation_custom.h
    internal/connection_impl.cc
    internal/connection_impl.h
    internal/default_options.cc
    internal/default_options.h
    internal/retry_traits.h
    internal/tracing_connection.cc
    internal/tracing_connection.h
    job_options.h
    read_arrow_response.h
    read_options.h
    retry_policy.h)

set(bigquery_unified_deps
    # cmake-format: sort
    Arrow::arrow_static
    google-cloud-cpp-bigquery::common
    google-cloud-cpp::bigquery
    google-cloud-cpp::experimental-bigquerycontrol
    google-cloud-cpp::grpc_utils)

set(bigquery_unified_pkgconfig_deps
    # cmake-format: sort
    arrow google_cloud_cpp_bigquery google_cloud_cpp_bigquery_common
    google_cloud_cpp_bigquerycontrol)

bigquery_unified_library(
    google_cloud_cpp_bigquery_bigquery_unified
    bigquery_unified
    INSTALL
    INITIAL_YEAR
    "2024"
    LIBRARY_FILES
    ${bigquery_unified_library_files}
    DEPS
    ${bigquery_unified_deps}
    PKGCONFIG_DEPS
    ${bigquery_unified_pkgconfig_deps})

if (GOOGLE_CLOUD_CPP_BIGQUERY_WITH_MOCKS)
    add_library(google_cloud_cpp_bigquery_bigquery_unified_mocks INTERFACE)
    set(google_cloud_cpp_bigquery_bigquery_unified_mocks_hdrs
        # cmake-format: sort
        mocks/mock_connection.h)
    export_list_to_bazel(
        "google_cloud_cpp_bigquery_bigquery_unified_mocks.bzl"
        "google_cloud_cpp_bigquery_bigquery_unified_mocks_hdrs" YEAR "2024")

    # Add the prefix to each file name for cmake
    set(cmake_google_cloud_cpp_bigquery_bigquery_unified_mocks_hdrs)
    foreach (fname ${google_cloud_cpp_bigquery_bigquery_unified_mocks_hdrs})
        list(APPEND cmake_google_cloud_cpp_bigquery_bigquery_unified_mocks_hdrs
             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${fname}>")
    endforeach ()
    target_sources(
        google_cloud_cpp_bigquery_bigquery_unified_mocks
        INTERFACE ${cmake_google_cloud_cpp_bigquery_bigquery_unified_mocks_hdrs}
    )
    target_link_libraries(
        google_cloud_cpp_bigquery_bigquery_unified_mocks
        INTERFACE google-cloud-cpp-bigquery::bigquery_unified GTest::gmock
                  GTest::gtest)
    set_target_properties(
        google_cloud_cpp_bigquery_bigquery_unified_mocks
        PROPERTIES EXPORT_NAME
                   google-cloud-cpp-bigquery::bigquery_unified_mocks)
    target_include_directories(
        google_cloud_cpp_bigquery_bigquery_unified_mocks
        INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
                  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
                  $<INSTALL_INTERFACE:include>)
    set_target_properties(
        google_cloud_cpp_bigquery_bigquery_unified_mocks
        PROPERTIES EXPORT_NAME
                   "google-cloud-cpp-bigquery::bigquery_unified_mocks")
    target_compile_options(google_cloud_cpp_bigquery_bigquery_unified_mocks
                           INTERFACE ${GOOGLE_CLOUD_CPP_EXCEPTIONS_FLAG})
    add_library(google-cloud-cpp-bigquery::bigquery_unified_mocks ALIAS
                google_cloud_cpp_bigquery_bigquery_unified_mocks)

    google_cloud_cpp_bigquery_install_mocks(bigquery_unified
                                            "Google Cloud BigQuery Unified")
endif ()

add_subdirectory(testing_util)
add_subdirectory(integration_tests)
add_subdirectory(samples)

# Define the tests in a function so we have a new scope for variable names.
function (bigquery_unified_client_define_tests)
    # The tests require googletest to be installed. Force CMake to use the
    # config file for googletest (that is, the CMake file installed by
    # googletest itself), because the generic `FindGTest` module does not define
    # the GTest::gmock target, and the target names are also weird.
    find_package(GTest CONFIG REQUIRED)
    set(bigquery_unified_client_unit_tests
        # cmake-format: sort
        client_test.cc internal/connection_impl_test.cc
        internal/default_options_test.cc)

    # Export the list of unit tests to a .bzl file so we do not need to maintain
    # the list in two places.
    export_list_to_bazel("bigquery_unified_client_unit_tests.bzl"
                         "bigquery_unified_client_unit_tests" YEAR "2024")

    # Create a custom target so we can say "build all the tests"
    add_custom_target(bigquery_unified-client-tests)

    # Generate a target for each unit test.
    foreach (fname ${bigquery_unified_client_unit_tests})
        google_cloud_cpp_add_executable(target "bigquery_unified" "${fname}")
        target_link_libraries(
            ${target}
            PRIVATE google-cloud-cpp-bigquery::bigquery_unified
                    google_cloud_cpp_bigquery_bigquery_unified_testing
                    GTest::gmock_main GTest::gmock GTest::gtest)
        google_cloud_cpp_add_common_options(${target})
        add_test(NAME ${target} COMMAND ${target})
        add_dependencies(bigquery_unified-client-tests ${target})
    endforeach ()
endfunction ()

if (BUILD_TESTING)
    bigquery_unified_client_define_tests()
endif ()

if (BUILD_TESTING AND GOOGLE_CLOUD_CPP_ENABLE_CXX_EXCEPTIONS)
    add_executable(bigquery_unified_quickstart "quickstart/quickstart.cc")
    target_link_libraries(bigquery_unified_quickstart
                          PRIVATE google-cloud-cpp-bigquery::bigquery_unified)
    google_cloud_cpp_add_common_options(bigquery_unified_quickstart)
    add_test(
        NAME bigquery_unified_quickstart
        COMMAND cmake -P "${PROJECT_SOURCE_DIR}/cmake/quickstart-runner.cmake"
                $<TARGET_FILE:bigquery_unified_quickstart> GOOGLE_CLOUD_PROJECT)
    set_tests_properties(bigquery_unified_quickstart
                         PROPERTIES LABELS "integration-test;quickstart")
endif ()
