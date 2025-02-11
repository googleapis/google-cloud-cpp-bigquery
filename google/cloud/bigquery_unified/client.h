// Copyright 2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_CLIENT_H
#define GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_CLIENT_H

#include "google/cloud/bigquery_unified/connection.h"
#include "google/cloud/bigquery_unified/read_arrow_response.h"
#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/future.h"
#include "google/cloud/no_await_tag.h"
#include "google/cloud/options.h"
#include "google/cloud/status_or.h"
#include <google/cloud/bigquery/storage/v1/storage.pb.h>
#include <google/cloud/bigquery/v2/job.pb.h>
#include <memory>

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

///
/// BigQuery Unified API.
///
/// The BigQuery Unified API integrates various BigQuery services in order to
/// make it easier to use the features available across the individual BigQuery
/// services in combination with each other in an idiomatic C++ manner.
///
/// @par Equality
///
/// Instances of this class created via copy-construction or copy-assignment
/// always compare equal. Instances created with equal
/// `std::shared_ptr<*Connection>` objects compare equal. Objects that compare
/// equal share the same underlying resources.
///
/// @par Performance
///
/// Creating a new instance of this class is a relatively expensive operation,
/// new objects establish new connections to the service. In contrast,
/// copy-construction, move-construction, and the corresponding assignment
/// operations are relatively efficient as the copies share all underlying
/// resources.
///
/// @par Thread Safety
///
/// Concurrent access to different instances of this class, even if they compare
/// equal, is guaranteed to work. Two or more threads operating on the same
/// instance of this class is not guaranteed to work. Since copy-construction
/// and move-construction is a relatively efficient operation, consider using
/// such a copy when using this class from multiple threads.
///
class Client {
 public:
  explicit Client(std::shared_ptr<Connection> connection, Options opts = {});
  ~Client() = default;

  ///@{
  /// @name Copy and move support
  Client(Client const&) = default;
  Client& operator=(Client const&) = default;
  Client(Client&&) = default;
  Client& operator=(Client&&) = default;
  ///@}

  ///@{
  /// @name Equality
  friend bool operator==(Client const& a, Client const& b) {
    return a.connection_ == b.connection_;
  }
  friend bool operator!=(Client const& a, Client const& b) { return !(a == b); }
  ///@}

  // clang-format off
  ///
  /// Requests that a job be cancelled. Cancelled jobs may still incur costs.
  ///
  /// @param request Unary RPCs, such as the one wrapped by this
  ///     function, receive a single `request` proto message which includes all
  ///     the inputs for the RPC. In this case, the proto message is a
  ///     [google.cloud.bigquery.v2.CancelJobRequest].
  ///     Proto messages are converted to C++ classes by Protobuf, using the
  ///     [Protobuf mapping rules].
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return the result of the RPC. The response message type
  ///     ([google.cloud.bigquery.v2.Job])
  ///     is mapped to a C++ class using the [Protobuf mapping rules].
  ///     If the request fails, the [`StatusOr`] contains the error details.
  ///
  /// [Protobuf mapping rules]: https://protobuf.dev/reference/cpp/cpp-generated/
  /// [input iterator requirements]: https://en.cppreference.com/w/cpp/named_req/InputIterator
  /// [`std::string`]: https://en.cppreference.com/w/cpp/string/basic_string
  /// [`future`]: @ref google::cloud::future
  /// [`StatusOr`]: @ref google::cloud::StatusOr
  /// [`Status`]: @ref google::cloud::Status
  /// [google.cloud.bigquery.v2.CancelJobRequest]: @googleapis_reference_link{google/cloud/bigquery/v2/job.proto#L167}
  /// [google.cloud.bigquery.v2.JobReference]: @googleapis_reference_link{google/cloud/bigquery/v2/job_reference.proto#L27}
  /// [google.cloud.bigquery.v2.Job]: @googleapis_reference_link{google/cloud/bigquery/v2/job.proto#L125}
  ///
  // clang-format on
  future<StatusOr<google::cloud::bigquery::v2::Job>> CancelJob(
      google::cloud::bigquery::v2::CancelJobRequest const& request,
      Options opts = {});

  StatusOr<google::cloud::bigquery::v2::JobReference> CancelJob(
      google::cloud::NoAwaitTag,
      google::cloud::bigquery::v2::CancelJobRequest const& request,
      Options opts = {});

  future<StatusOr<google::cloud::bigquery::v2::Job>> CancelJob(
      google::cloud::bigquery::v2::JobReference const& job_reference,
      Options opts = {});

  // clang-format off
  ///
  /// Requests the deletion of the metadata of a job. This call returns when the
  /// job's metadata is deleted.
  ///
  /// @param request Unary RPCs, such as the one wrapped by this
  ///     function, receive a single `request` proto message which includes all
  ///     the inputs for the RPC. In this case, the proto message is a
  ///     [google.cloud.bigquery.v2.DeleteJobRequest].
  ///     Proto messages are converted to C++ classes by Protobuf, using the
  ///     [Protobuf mapping rules].
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return a [`Status`] object. If the request failed, the
  ///     status contains the details of the failure.
  ///
  /// [Protobuf mapping rules]: https://protobuf.dev/reference/cpp/cpp-generated/
  /// [input iterator requirements]: https://en.cppreference.com/w/cpp/named_req/InputIterator
  /// [`std::string`]: https://en.cppreference.com/w/cpp/string/basic_string
  /// [`future`]: @ref google::cloud::future
  /// [`StatusOr`]: @ref google::cloud::StatusOr
  /// [`Status`]: @ref google::cloud::Status
  /// [google.cloud.bigquery.v2.DeleteJobRequest]: @googleapis_reference_link{google/cloud/bigquery/v2/job.proto#L227}
  ///
  // clang-format on
  Status DeleteJob(google::cloud::bigquery::v2::DeleteJobRequest const& request,
                   Options opts = {});

  // clang-format off
  ///
  /// Returns information about a specific job. Job information is available for
  /// a six month period after creation. Requires that you're the person who ran
  /// the job, or have the Is Owner project role.
  ///
  /// @param request Unary RPCs, such as the one wrapped by this
  ///     function, receive a single `request` proto message which includes all
  ///     the inputs for the RPC. In this case, the proto message is a
  ///     [google.cloud.bigquery.v2.GetJobRequest].
  ///     Proto messages are converted to C++ classes by Protobuf, using the
  ///     [Protobuf mapping rules].
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return the result of the RPC. The response message type
  ///     ([google.cloud.bigquery.v2.Job])
  ///     is mapped to a C++ class using the [Protobuf mapping rules].
  ///     If the request fails, the [`StatusOr`] contains the error details.
  ///
  /// [Protobuf mapping rules]: https://protobuf.dev/reference/cpp/cpp-generated/
  /// [input iterator requirements]: https://en.cppreference.com/w/cpp/named_req/InputIterator
  /// [`std::string`]: https://en.cppreference.com/w/cpp/string/basic_string
  /// [`future`]: @ref google::cloud::future
  /// [`StatusOr`]: @ref google::cloud::StatusOr
  /// [`Status`]: @ref google::cloud::Status
  /// [google.cloud.bigquery.v2.GetJobRequest]: @googleapis_reference_link{google/cloud/bigquery/v2/job.proto#L197}
  /// [google.cloud.bigquery.v2.Job]: @googleapis_reference_link{google/cloud/bigquery/v2/job.proto#L125}
  ///
  // clang-format on
  StatusOr<google::cloud::bigquery::v2::Job> GetJob(
      google::cloud::bigquery::v2::GetJobRequest const& request,
      Options opts = {});

  // clang-format off
  ///
  /// Lists all jobs that you started in the specified project. Job information
  /// is available for a six month period after creation. The job list is sorted
  /// in reverse chronological order, by job creation time. Requires the Can View
  /// project role, or the Is Owner project role if you set the allUsers
  /// property.
  ///
  /// @param request Unary RPCs, such as the one wrapped by this
  ///     function, receive a single `request` proto message which includes all
  ///     the inputs for the RPC. In this case, the proto message is a
  ///     [google.cloud.bigquery.v2.ListJobsRequest].
  ///     Proto messages are converted to C++ classes by Protobuf, using the
  ///     [Protobuf mapping rules].
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return a [StreamRange](@ref google::cloud::StreamRange)
  ///     to iterate of the results. See the documentation of this type for
  ///     details. In brief, this class has `begin()` and `end()` member
  ///     functions returning a iterator class meeting the
  ///     [input iterator requirements]. The value type for this iterator is a
  ///     [`StatusOr`] as the iteration may fail even after some values are
  ///     retrieved successfully, for example, if there is a network disconnect.
  ///     An empty set of results does not indicate an error, it indicates
  ///     that there are no resources meeting the request criteria.
  ///     On a successful iteration the `StatusOr<T>` contains elements of type
  ///     [google.cloud.bigquery.v2.ListFormatJob], or rather,
  ///     the C++ class generated by Protobuf from that type. Please consult the
  ///     Protobuf documentation for details on the [Protobuf mapping rules].
  ///
  /// [Protobuf mapping rules]: https://protobuf.dev/reference/cpp/cpp-generated/
  /// [input iterator requirements]: https://en.cppreference.com/w/cpp/named_req/InputIterator
  /// [`std::string`]: https://en.cppreference.com/w/cpp/string/basic_string
  /// [`future`]: @ref google::cloud::future
  /// [`StatusOr`]: @ref google::cloud::StatusOr
  /// [`Status`]: @ref google::cloud::Status
  /// [google.cloud.bigquery.v2.ListFormatJob]: @googleapis_reference_link{google/cloud/bigquery/v2/job.proto#L320}
  /// [google.cloud.bigquery.v2.ListJobsRequest]: @googleapis_reference_link{google/cloud/bigquery/v2/job.proto#L244}
  ///
  // clang-format on
  StreamRange<google::cloud::bigquery::v2::ListFormatJob> ListJobs(
      google::cloud::bigquery::v2::ListJobsRequest request, Options opts = {});

  // clang-format off
  ///
  /// Starts a new asynchronous job.
  ///
  /// Unless `bigquery_unified::BillingProjectOption` is set, the billing
  /// project is determined by interrogating the provided `Job`.
  ///
  /// @param job Unary RPCs, such as the one wrapped by this
  ///     function, receive a single `request` proto message which includes all
  ///     the inputs for the RPC. In this case, the proto message is a
  ///     [google.cloud.bigquery.v2.Job].
  ///     Proto messages are converted to C++ classes by Protobuf, using the
  ///     [Protobuf mapping rules].
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return the result of the RPC. The response message type
  ///     ([google.cloud.bigquery.v2.Job])
  ///     is mapped to a C++ class using the [Protobuf mapping rules].
  ///     If the request fails, the [`StatusOr`] contains the error details.
  ///
  /// [Protobuf mapping rules]: https://protobuf.dev/reference/cpp/cpp-generated/
  /// [input iterator requirements]: https://en.cppreference.com/w/cpp/named_req/InputIterator
  /// [`std::string`]: https://en.cppreference.com/w/cpp/string/basic_string
  /// [`future`]: @ref google::cloud::future
  /// [`StatusOr`]: @ref google::cloud::StatusOr
  /// [`Status`]: @ref google::cloud::Status
  /// [google.cloud.bigquery.v2.Job]: @googleapis_reference_link{google/cloud/bigquery/v2/job.proto#L125}
  /// [google.cloud.bigquery.v2.JobReference]: @googleapis_reference_link{google/cloud/bigquery/v2/job_reference.proto#L27}
  ///
  // clang-format on
  future<StatusOr<google::cloud::bigquery::v2::Job>> InsertJob(
      google::cloud::bigquery::v2::Job const& job, Options opts = {});

  StatusOr<google::cloud::bigquery::v2::JobReference> InsertJob(
      google::cloud::NoAwaitTag, google::cloud::bigquery::v2::Job const& job,
      Options opts = {});

  future<StatusOr<google::cloud::bigquery::v2::Job>> InsertJob(
      google::cloud::bigquery::v2::JobReference const& job_reference,
      Options opts = {});

  // clang-format off
  ///
  /// Reads data in the Apache Arrow RecordBatch format from BigQuery.
  ///
  /// Unless `bigquery_unified::BillingProjectOption` is set, the billing
  /// project is assumed to be the same as the project owning the table.
  /// Unless `bigquery_unified::MaxReadStreamsOption` is set, the service
  /// suggested number of readers will be present in the response.
  /// Setting `bigquery_unified::MaxReadStreamsOption` is required to guarantee
  /// ordering when reading results from ordered queries.
  ///
  /// @param job Unary RPCs, such as the one wrapped by this
  ///     function, receive a single `request` proto message which includes all
  ///     the inputs for the RPC. In this case, the proto message is a
  ///     [google.cloud.bigquery.v2.Job].
  ///     Proto messages are converted to C++ classes by Protobuf, using the
  ///     [Protobuf mapping rules].
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return the result of the RPC. The response type ([ReadArrowResponse])
  ///     contains one or more `readers` that can be used to iterate over the
  ///     data read.
  ///     If the request fails, the [`StatusOr`] contains the error details.
  ///
  /// [`future`]: @ref google::cloud::future
  /// [`StatusOr`]: @ref google::cloud::StatusOr
  /// [google.cloud.bigquery.v2.Job]: @googleapis_reference_link{google/cloud/bigquery/v2/job.proto#L125}
  /// [google.cloud.bigquery.v2.JobReference]: @googleapis_reference_link{google/cloud/bigquery/v2/job_reference.proto#L27}
  /// [google.cloud.bigquery.v2.TableReference]: @googleapis_reference_link{google/cloud/bigquery/v2/table_reference.proto#L25}
  ///
  // clang-format on
  StatusOr<ReadArrowResponse> ReadArrow(
      google::cloud::bigquery::v2::Job const& job, Options opts = {});
  StatusOr<ReadArrowResponse> ReadArrow(
      google::cloud::bigquery::v2::JobReference const& job_reference,
      Options opts = {});
  StatusOr<ReadArrowResponse> ReadArrow(
      google::cloud::bigquery::v2::TableReference const& table_reference,
      Options opts = {});

  // clang-format off
  ///
  /// Reads data in the Apache Arrow RecordBatch format from BigQuery.
  ///
  /// This ReadArrow overload allows for full customization of the read session,
  /// except for AVRO format or AVRO serialization options which are ignored.
  /// All bigquery_unified::*Option are ignored except for:
  ///   - bigquery_unified::BackoffPolicyOption
  ///   - bigquery_unified::IdempotencyPolicyOption
  ///   - bigquery_unified::PollingPolicyOption
  ///   - bigquery_unified::RetryPolicyOption
  ///
  // clang-format on
  StatusOr<ReadArrowResponse> ReadArrow(
      google::cloud::bigquery::storage::v1::CreateReadSessionRequest const&
          read_session_request,
      Options opts = {});

 private:
  StatusOr<ReadArrowResponse> ReadArrowHelper(
      google::cloud::bigquery::v2::TableReference const& table_reference,
      std::string billing_project, Options opts);

  std::shared_ptr<Connection> connection_;
  Options options_;
};

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_CLIENT_H
