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

#include "google/cloud/bigquery_unified/internal/tracing_connection.h"
#include "google/cloud/internal/opentelemetry.h"
#include "google/cloud/internal/traced_stream_range.h"


namespace google::cloud::bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

TracingConnection::TracingConnection(std::shared_ptr<bigquery_unified::Connection>
                                         child)
    : child_(std::move(child)) {}

future<StatusOr<google::cloud::bigquery::v2::Job>> TracingConnection::CancelJob(
      google::cloud::bigquery::v2::CancelJobRequest const& request,
      Options opts) {
    auto span = internal::MakeSpan(
      "bigquery_unified::Connection::CancelJob");
    internal::OTelScope scope(span);
    return internal::EndSpan(std::move(span), child_->CancelJob(request, opts));
}

StatusOr<google::cloud::bigquery::v2::JobReference> TracingConnection::CancelJob(
    google::cloud::NoAwaitTag,
    google::cloud::bigquery::v2::CancelJobRequest const& request,
    Options opts) {
    auto span = internal::MakeSpan(
    "bigquery_unified::Connection::CancelJob");
    auto scope = opentelemetry::trace::Scope(span);
    return internal::EndSpan(*span, child_->CancelJob(NoAwaitTag{}, request, opts));
}

future<StatusOr<google::cloud::bigquery::v2::Job>> TracingConnection::CancelJob(
    google::cloud::bigquery::v2::JobReference const& job_reference,
    Options opts) {
    auto span = internal::MakeSpan(
      "bigquery_unified::Connection::CancelJob");
    internal::OTelScope scope(span);
    return internal::EndSpan(std::move(span), child_->CancelJob(job_reference, opts));
    }

StatusOr<google::cloud::bigquery::v2::Job> TracingConnection::GetJob(
    google::cloud::bigquery::v2::GetJobRequest const& request,
    Options opts) {
    auto span = internal::MakeSpan(
    "bigquery_unified::Connection::GetJob");
    auto scope = opentelemetry::trace::Scope(span);
    return internal::EndSpan(*span, child_->GetJob(request, opts));
    }

future<StatusOr<google::cloud::bigquery::v2::Job>> TracingConnection::InsertJob(
    google::cloud::bigquery::v2::Job const& job, Options opts) {
    auto span = internal::MakeSpan(
      "bigquery_unified::Connection::InsertJob");
    internal::OTelScope scope(span);
    return internal::EndSpan(std::move(span), child_->InsertJob(job, opts));
    }

StatusOr<google::cloud::bigquery::v2::JobReference> TracingConnection::InsertJob(
    google::cloud::NoAwaitTag, google::cloud::bigquery::v2::Job const& job,
    Options opts) {
    auto span = internal::MakeSpan(
    "bigquery_unified::Connection::InsertJob");
    auto scope = opentelemetry::trace::Scope(span);
    return internal::EndSpan(*span, child_->InsertJob(NoAwaitTag{}, job, opts));
    }

future<StatusOr<google::cloud::bigquery::v2::Job>> TracingConnection::InsertJob(
    google::cloud::bigquery::v2::JobReference const& job_reference,
    Options opts) {
    auto span = internal::MakeSpan(
      "bigquery_unified::Connection::InsertJob");
    internal::OTelScope scope(span);
    return internal::EndSpan(std::move(span), child_->InsertJob(job_reference, opts));
    }

Status TracingConnection::DeleteJob(google::cloud::bigquery::v2::DeleteJobRequest const& request,
                Options opts) {
    auto span = internal::MakeSpan(
    "bigquery_unified::Connection::DeleteJob");
    auto scope = opentelemetry::trace::Scope(span);
    return internal::EndSpan(*span, child_->DeleteJob(request, opts));
}

StreamRange<google::cloud::bigquery::v2::ListFormatJob> TracingConnection::ListJobs(
    google::cloud::bigquery::v2::ListJobsRequest request,
    Options opts) {
    auto span =
      internal::MakeSpan("bigquery_unified::Connection::ListJobs");
    internal::OTelScope scope(span);
    auto sr = child_->ListJobs(std::move(request), opts);
    return internal::MakeTracedStreamRange<
        google::cloud::bigquery::v2::ListFormatJob>(std::move(span),
                                                  std::move(sr));
    }

StatusOr<bigquery_unified::ReadArrowResponse> TracingConnection::ReadArrow(
    google::cloud::bigquery::storage::v1::CreateReadSessionRequest const&
        read_session,
    Options opts) {
    // Not add span tracing for now, will add it after discussion.
    return child_->ReadArrow(read_session, opts);
    }
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

std::shared_ptr<bigquery_unified::Connection>
MakeTracingConnection(
    std::shared_ptr<bigquery_unified::Connection> conn) {
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
  if (internal::TracingEnabled(conn->options())) {
    conn = std::make_shared<TracingConnection>(std::move(conn));
  }
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
  return conn;
}

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_internal