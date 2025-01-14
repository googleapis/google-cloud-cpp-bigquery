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

#include "google/cloud/bigquery_unified/client.h"
#include "google/cloud/internal/make_status.h"
#include "google/cloud/internal/pagination_range.h"
#include "google/cloud/options.h"

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

Client::Client(std::shared_ptr<Connection> connection, Options opts)
    : connection_(std::move(connection)),
      options_(
          internal::MergeOptions(std::move(opts), connection_->options())) {}

future<StatusOr<google::cloud::bigquery::v2::JobCancelResponse>>
Client::CancelJob(google::cloud::bigquery::v2::CancelJobRequest const& request,
                  Options opts) {
  return make_ready_future<
      StatusOr<google::cloud::bigquery::v2::JobCancelResponse>>(
      internal::UnimplementedError("not implemented"));
}

StatusOr<google::cloud::bigquery::v2::JobReference> Client::CancelJob(
    google::cloud::NoAwaitTag,
    google::cloud::bigquery::v2::CancelJobRequest const& request,
    Options opts) {
  return internal::UnimplementedError("not implemented");
}

future<StatusOr<google::cloud::bigquery::v2::JobCancelResponse>>
Client::CancelJob(
    google::cloud::bigquery::v2::JobReference const& job_reference,
    Options opts) {
  return make_ready_future<
      StatusOr<google::cloud::bigquery::v2::JobCancelResponse>>(
      internal::UnimplementedError("not implemented"));
}

StatusOr<google::cloud::bigquery::v2::Job> Client::GetJob(
    google::cloud::bigquery::v2::GetJobRequest const& request, Options opts) {
  return connection_->GetJob(request,
                             internal::MergeOptions(std::move(opts), options_));
}

Status Client::DeleteJob(
    google::cloud::bigquery::v2::DeleteJobRequest const& request,
    Options opts) {
  return connection_->DeleteJob(
      request, internal::MergeOptions(std::move(opts), options_));
}

StreamRange<google::cloud::bigquery::v2::ListFormatJob> Client::ListJobs(
    google::cloud::bigquery::v2::ListJobsRequest request, Options opts) {
  return internal::MakeUnimplementedPaginationRange<
      StreamRange<google::cloud::bigquery::v2::ListFormatJob>>();
}

future<StatusOr<google::cloud::bigquery::v2::Job>> Client::InsertJob(
    google::cloud::bigquery::v2::InsertJobRequest const& request,
    Options opts) {
  return make_ready_future<StatusOr<google::cloud::bigquery::v2::Job>>(
      internal::UnimplementedError("not implemented"));
}

StatusOr<google::cloud::bigquery::v2::JobReference> Client::InsertJob(
    google::cloud::NoAwaitTag,
    google::cloud::bigquery::v2::InsertJobRequest const& request,
    Options opts) {
  return internal::UnimplementedError("not implemented");
}

future<StatusOr<google::cloud::bigquery::v2::Job>> Client::InsertJob(
    google::cloud::bigquery::v2::JobReference const& job_reference,
    Options opts) {
  return make_ready_future<StatusOr<google::cloud::bigquery::v2::Job>>(
      internal::UnimplementedError("not implemented"));
}

StatusOr<ReadArrowResponse> Client::ReadArrow(
    google::cloud::bigquery::v2::Job const& job, Options opts) {
  return internal::UnimplementedError("not implemented");
}
StatusOr<ReadArrowResponse> Client::ReadArrow(
    google::cloud::bigquery::v2::JobReference const& job_reference,
    Options opts) {
  return internal::UnimplementedError("not implemented");
}
StatusOr<ReadArrowResponse> Client::ReadArrow(
    google::cloud::bigquery::v2::TableReference const& table_reference,
    Options opts) {
  return internal::UnimplementedError("not implemented");
}

StatusOr<ReadArrowResponse> Client::ReadArrow(
    google::cloud::bigquery::storage::v1::CreateReadSessionRequest const&
        read_session,
    Options opts) {
  return internal::UnimplementedError("not implemented");
}

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified
