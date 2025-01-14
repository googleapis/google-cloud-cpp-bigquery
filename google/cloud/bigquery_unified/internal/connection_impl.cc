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

#include "google/cloud/bigquery_unified/internal/connection_impl.h"
#include "google/cloud/bigquerycontrol/v2/internal/job_option_defaults.h"
#include "google/cloud/bigquerycontrol/v2/internal/job_rest_connection_impl.h"
#include "google/cloud/bigquerycontrol/v2/internal/job_rest_stub_factory.h"
#include "google/cloud/bigquerycontrol/v2/internal/job_tracing_connection.h"
#include "google/cloud/background_threads.h"
#include "google/cloud/internal/rest_background_threads_impl.h"

namespace google::cloud::bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

ConnectionImpl::ConnectionImpl(
    std::shared_ptr<google::cloud::bigquerycontrol_v2::JobServiceConnection>
        job_connection,
    google::cloud::Options job_options,
    std::shared_ptr<bigquerycontrol_v2_internal::JobServiceRestStub> job_stub,
    google::cloud::Options options)
    : job_connection_(std::move(job_connection)),
      job_stub_(std::move(job_stub)),
      job_options_(std::move(job_options)),
      options_(std::move(options)) {}

StatusOr<google::cloud::bigquery::v2::Job> ConnectionImpl::GetJob(
    google::cloud::bigquery::v2::GetJobRequest const& request, Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  return job_connection_->GetJob(request);
}

Status ConnectionImpl::DeleteJob(
    google::cloud::bigquery::v2::DeleteJobRequest const& request,
    Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  return job_connection_->DeleteJob(request);
}

std::shared_ptr<bigquery_unified::Connection> MakeDefaultConnectionImpl(
    Options options) {
  auto job_options =
      bigquerycontrol_v2_internal::JobServiceDefaultOptions(options);
  auto job_background = std::make_unique<
      rest_internal::AutomaticallyCreatedRestBackgroundThreads>();
  auto job_stub =
      bigquerycontrol_v2_internal::CreateDefaultJobServiceRestStub(job_options);
  std::shared_ptr<bigquerycontrol_v2::JobServiceConnection> job_connection =
      bigquerycontrol_v2_internal::MakeJobServiceTracingConnection(
          std::make_shared<
              bigquerycontrol_v2_internal::JobServiceRestConnectionImpl>(
              std::move(job_background), job_stub, job_options));

  // TODO: We should probably add a TracingConnection decorator in order to
  //  associate all the various rpcs that are called as part of these
  //  operations.
  return std::make_shared<bigquery_unified_internal::ConnectionImpl>(
      std::move(job_connection), std::move(job_options), std::move(job_stub),
      std::move(options));
}

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_internal
