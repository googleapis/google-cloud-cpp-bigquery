// Copyright 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_INTERNAL_TRACING_CONNECTIONL_H
#define GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_INTERNAL_TRACING_CONNECTIONL_H

#include "google/cloud/bigquery_unified/connection.h"
#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/bigquerycontrol/v2/job_connection.h"
#include "google/cloud/bigquery/storage/v1/bigquery_read_connection.h"
#include <memory>

namespace google::cloud::bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

class TracingConnection : public bigquery_unified::Connection {
 public:
  ~TracingConnection() override = default;

  explicit TracingConnection(std::shared_ptr<bigquery_unified::Connection>
                                 child);

  Options options() override { return child_->options(); }

  future<StatusOr<google::cloud::bigquery::v2::Job>> CancelJob(
      google::cloud::bigquery::v2::CancelJobRequest const& request,
      Options opts) override;

  StatusOr<google::cloud::bigquery::v2::JobReference> CancelJob(
      google::cloud::NoAwaitTag,
      google::cloud::bigquery::v2::CancelJobRequest const& request,
      Options opts) override;

  future<StatusOr<google::cloud::bigquery::v2::Job>> CancelJob(
      google::cloud::bigquery::v2::JobReference const& job_reference,
      Options opts) override;

  StatusOr<google::cloud::bigquery::v2::Job> GetJob(
      google::cloud::bigquery::v2::GetJobRequest const& request,
      Options opts) override;

  future<StatusOr<google::cloud::bigquery::v2::Job>> InsertJob(
      google::cloud::bigquery::v2::Job const& job, Options opts) override;

  StatusOr<google::cloud::bigquery::v2::JobReference> InsertJob(
      google::cloud::NoAwaitTag, google::cloud::bigquery::v2::Job const& job,
      Options opts) override;

  future<StatusOr<google::cloud::bigquery::v2::Job>> InsertJob(
      google::cloud::bigquery::v2::JobReference const& job_reference,
      Options opts) override;

  Status DeleteJob(google::cloud::bigquery::v2::DeleteJobRequest const& request,
                   Options opts) override;

  StreamRange<google::cloud::bigquery::v2::ListFormatJob> ListJobs(
      google::cloud::bigquery::v2::ListJobsRequest request,
      Options opts) override;

  StatusOr<bigquery_unified::ReadArrowResponse> ReadArrow(
      google::cloud::bigquery::storage::v1::CreateReadSessionRequest const&
          read_session,
      Options opts) override;

 private:
  std::shared_ptr<bigquery_unified::Connection> child_;
};

/**
 * Conditionally applies the tracing decorator to the given connection.
 *
 * The connection is only decorated if tracing is enabled (as determined by the
 * connection's options).
 */
std::shared_ptr<bigquery_unified::Connection>
MakeTracingConnection(
    std::shared_ptr<bigquery_unified::Connection> conn);

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_internal 

#endif // GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_INTERNAL_TRACING_CONNECTIONL_H