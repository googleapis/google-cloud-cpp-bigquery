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

#ifndef GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_INTERNAL_CONNECTION_IMPL_H
#define GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_INTERNAL_CONNECTION_IMPL_H

#include "google/cloud/bigquery_unified/connection.h"
#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/bigquerycontrol/v2/internal/job_rest_stub.h"
#include "google/cloud/bigquerycontrol/v2/job_connection.h"

namespace google::cloud::bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

class ConnectionImpl : public bigquery_unified::Connection {
 public:
  ConnectionImpl(
      std::shared_ptr<google::cloud::bigquerycontrol_v2::JobServiceConnection>
          job_connection,
      google::cloud::Options job_options,
      std::shared_ptr<bigquerycontrol_v2_internal::JobServiceRestStub> job_stub,
      google::cloud::Options options);

  ~ConnectionImpl() override = default;

  Options options() override { return options_; }

  StatusOr<google::cloud::bigquery::v2::Job> GetJob(
      google::cloud::bigquery::v2::GetJobRequest const& request,
      Options opts) override;

  Status DeleteJob(google::cloud::bigquery::v2::DeleteJobRequest const& request,
                   Options opts) override;

 private:
  std::shared_ptr<bigquerycontrol_v2::JobServiceConnection> job_connection_;
  std::shared_ptr<bigquerycontrol_v2_internal::JobServiceRestStub> job_stub_;
  Options job_options_;
  Options options_;
};

std::shared_ptr<bigquery_unified::Connection> MakeDefaultConnectionImpl(
    Options options);

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_internal

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_INTERNAL_CONNECTION_IMPL_H
