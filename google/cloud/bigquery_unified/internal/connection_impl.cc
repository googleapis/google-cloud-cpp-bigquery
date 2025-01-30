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
#include "google/cloud/bigquery/storage/v1/internal/bigquery_read_connection_impl.h"
#include "google/cloud/bigquery/storage/v1/internal/bigquery_read_option_defaults.h"
#include "google/cloud/bigquery/storage/v1/internal/bigquery_read_stub_factory.h"
#include "google/cloud/bigquery/storage/v1/internal/bigquery_read_tracing_connection.h"
#include "google/cloud/bigquery_unified/idempotency_policy.h"
#include "google/cloud/bigquery_unified/internal/arrow_reader.h"
#include "google/cloud/bigquery_unified/internal/async_rest_long_running_operation_custom.h"
#include "google/cloud/bigquery_unified/internal/default_options.h"
#include "google/cloud/bigquery_unified/internal/tracing_connection.h"
#include "google/cloud/bigquery_unified/job_options.h"
#include "google/cloud/bigquery_unified/read_options.h"
#include "google/cloud/bigquery_unified/retry_policy.h"
#include "google/cloud/bigquerycontrol/v2/internal/job_option_defaults.h"
#include "google/cloud/bigquerycontrol/v2/internal/job_rest_connection_impl.h"
#include "google/cloud/bigquerycontrol/v2/internal/job_rest_stub_factory.h"
#include "google/cloud/bigquerycontrol/v2/internal/job_tracing_connection.h"
#include "google/cloud/background_threads.h"
#include "google/cloud/grpc_options.h"
#include "google/cloud/internal/rest_background_threads_impl.h"
#include "google/cloud/internal/rest_retry_loop.h"

namespace google::cloud::bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
namespace {

std::unique_ptr<bigquery_unified::RetryPolicy> retry_policy(
    Options const& options) {
  return options.get<bigquery_unified::RetryPolicyOption>()->clone();
}

std::unique_ptr<BackoffPolicy> backoff_policy(Options const& options) {
  return options.get<bigquery_unified::BackoffPolicyOption>()->clone();
}

std::unique_ptr<bigquery_unified::IdempotencyPolicy> idempotency_policy(
    Options const& options) {
  return options.get<bigquery_unified::IdempotencyPolicyOption>()->clone();
}

std::unique_ptr<PollingPolicy> polling_policy(Options const& options) {
  return options.get<bigquery_unified::PollingPolicyOption>()->clone();
}

}  // namespace

ConnectionImpl::ConnectionImpl(
    std::shared_ptr<google::cloud::bigquery_storage_v1::BigQueryReadConnection>
        read_connection,
    std::shared_ptr<google::cloud::bigquerycontrol_v2::JobServiceConnection>
        job_connection,
    google::cloud::Options read_options, google::cloud::Options job_options,
    std::shared_ptr<bigquerycontrol_v2_internal::JobServiceRestStub> job_stub,
    std::unique_ptr<google::cloud::BackgroundThreads> background,
    google::cloud::Options options)
    : read_connection_(std::move(read_connection)),
      job_connection_(std::move(job_connection)),
      job_stub_(std::move(job_stub)),
      read_options_(std::move(read_options)),
      job_options_(std::move(job_options)),
      background_(std::move(background)),
      options_(std::move(options)) {}

future<StatusOr<google::cloud::bigquery::v2::Job>> ConnectionImpl::CancelJob(
    google::cloud::bigquery::v2::CancelJobRequest const& request,
    Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  auto current_options = google::cloud::internal::SaveCurrentOptions();

  auto idempotency = idempotency_policy(*current_options)
                         ->CancelJob(request, *current_options);
  auto cancel_response = rest_internal::RestRetryLoop(
      retry_policy(*current_options), backoff_policy(*current_options),
      std::move(idempotency),
      [stub = job_stub_](
          rest_internal::RestContext& rest_context, Options const& options,
          google::cloud::bigquery::v2::CancelJobRequest const& request) {
        return stub->CancelJob(rest_context, options, request);
      },
      *current_options, request, __func__);

  if (!cancel_response) {
    return make_ready_future(
        StatusOr<google::cloud::bigquery::v2::Job>(cancel_response.status()));
  }

  return JobPoll(cancel_response->job(), current_options, "CancelJob");
}

StatusOr<google::cloud::bigquery::v2::JobReference> ConnectionImpl::CancelJob(
    google::cloud::NoAwaitTag,
    google::cloud::bigquery::v2::CancelJobRequest const& request,
    Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  auto current_options = google::cloud::internal::SaveCurrentOptions();

  auto cancel_response = job_connection_->CancelJob(request);
  if (!cancel_response) {
    return cancel_response.status();
  }
  return cancel_response->job().job_reference();
}

future<StatusOr<google::cloud::bigquery::v2::Job>> ConnectionImpl::CancelJob(
    google::cloud::bigquery::v2::JobReference const& job_reference,
    Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  auto current_options = google::cloud::internal::SaveCurrentOptions();

  google::cloud::bigquery::v2::GetJobRequest get_job_request;
  get_job_request.set_project_id(job_reference.project_id());
  get_job_request.set_job_id(job_reference.job_id());
  get_job_request.set_location(job_reference.location().value());
  auto get_job_response = job_connection_->GetJob(get_job_request);
  if (!get_job_response) {
    return make_ready_future(
        StatusOr<google::cloud::bigquery::v2::Job>(get_job_response.status()));
  }

  return JobPoll(*get_job_response, current_options, "CancelJob");
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

StatusOr<google::cloud::bigquery::v2::Job> ConnectionImpl::GetJob(
    google::cloud::bigquery::v2::GetJobRequest const& request, Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  return job_connection_->GetJob(request);
}

future<StatusOr<google::cloud::bigquery::v2::Job>> ConnectionImpl::JobPoll(
    google::cloud::bigquery::v2::Job const& operation,
    std::shared_ptr<Options const> const& current_options,
    std::string operation_name) {
  return bigquery_unified_internal::AsyncRestAwaitLongRunningOperation<
      google::cloud::bigquery::v2::Job, google::cloud::bigquery::v2::Job,
      google::cloud::bigquery::v2::GetJobRequest,
      google::cloud::bigquery::v2::CancelJobRequest>(
      background_->cq(), current_options, operation,
      [stub = job_stub_](
          CompletionQueue& cq,
          std::unique_ptr<rest_internal::RestContext> context,
          google::cloud::internal::ImmutableOptions options,
          google::cloud::bigquery::v2::GetJobRequest const& request)
          -> future<StatusOr<google::cloud::bigquery::v2::Job>> {
        return make_ready_future(
            stub->GetJob(*std::move(context), *std::move(options), request));
      },
      [stub = job_stub_](
          CompletionQueue& cq,
          std::unique_ptr<rest_internal::RestContext> context,
          google::cloud::internal::ImmutableOptions options,
          google::cloud::bigquery::v2::CancelJobRequest const& request)
          -> future<Status> {
        auto cancel_response =
            stub->CancelJob(*std::move(context), *std::move(options), request);
        if (!cancel_response) {
          return make_ready_future(std::move(cancel_response).status());
        }
        return make_ready_future(Status{});
      },
      [](StatusOr<google::cloud::bigquery::v2::Job> op, std::string const&) {
        return op;
      },
      polling_policy(*current_options), __func__,
      [](google::cloud::bigquery::v2::Job const& op) {
        return op.status().state() == "DONE";
      },
      [ref = operation.job_reference()](
          std::string const&, google::cloud::bigquery::v2::GetJobRequest& r) {
        r.set_project_id(ref.project_id());
        r.set_job_id(ref.job_id());
        r.set_location(ref.location().value());
      },
      [ref = operation.job_reference()](
          std::string const&,
          google::cloud::bigquery::v2::CancelJobRequest& r) {
        r.set_project_id(ref.project_id());
        r.set_job_id(ref.job_id());
        r.set_location(ref.location().value());
      },
      [op_name = std::move(operation_name)](
          StatusOr<google::cloud::bigquery::v2::Job> const&) {
        return op_name;
      });
}

std::string DetermineBillingProject(
    google::cloud::bigquery::v2::Job const& job) {
  if (job.configuration().job_type() == "QUERY") {
    return job.configuration().query().destination_table().project_id();
  } else if (job.configuration().job_type() == "COPY") {
    return job.configuration().copy().destination_table().project_id();
  } else if (job.configuration().job_type() == "LOAD") {
    return job.configuration().load().destination_table().project_id();
  } else if (job.configuration().job_type() == "EXTRACT") {
    if (job.configuration().extract().has_source_table()) {
      return job.configuration().extract().source_table().project_id();
    }
    if (job.configuration().extract().has_source_model()) {
      return job.configuration().extract().source_model().project_id();
    }
  }
  return "";
}

future<StatusOr<google::cloud::bigquery::v2::Job>> ConnectionImpl::InsertJob(
    google::cloud::bigquery::v2::Job const& job, Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  auto current_options = google::cloud::internal::SaveCurrentOptions();
  google::cloud::bigquery::v2::InsertJobRequest insert_request;
  auto const billing_project =
      current_options->has<bigquery_unified::BillingProjectOption>()
          ? current_options->get<bigquery_unified::BillingProjectOption>()
          : DetermineBillingProject(job);

  insert_request.set_project_id(billing_project);
  *insert_request.mutable_job() = job;
  auto insert_response = job_connection_->InsertJob(insert_request);

  if (!insert_response) {
    return make_ready_future(
        StatusOr<google::cloud::bigquery::v2::Job>(insert_response.status()));
  }

  return JobPoll(*insert_response, current_options, "InsertJob");
}

StatusOr<google::cloud::bigquery::v2::JobReference> ConnectionImpl::InsertJob(
    google::cloud::NoAwaitTag, google::cloud::bigquery::v2::Job const& job,
    Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  auto current_options = google::cloud::internal::SaveCurrentOptions();
  google::cloud::bigquery::v2::InsertJobRequest insert_request;
  auto const billing_project =
      current_options->has<bigquery_unified::BillingProjectOption>()
          ? current_options->get<bigquery_unified::BillingProjectOption>()
          : DetermineBillingProject(job);

  insert_request.set_project_id(billing_project);
  *insert_request.mutable_job() = job;
  auto insert_response = job_connection_->InsertJob(insert_request);
  if (!insert_response) {
    return insert_response.status();
  }
  return insert_response->job_reference();
}

future<StatusOr<google::cloud::bigquery::v2::Job>> ConnectionImpl::InsertJob(
    google::cloud::bigquery::v2::JobReference const& job_reference,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  auto current_options = google::cloud::internal::SaveCurrentOptions();

  google::cloud::bigquery::v2::GetJobRequest get_job_request;
  get_job_request.set_project_id(job_reference.project_id());
  get_job_request.set_job_id(job_reference.job_id());
  get_job_request.set_location(job_reference.location().value());
  auto get_job_response = job_connection_->GetJob(get_job_request);
  if (!get_job_response) {
    return make_ready_future(
        StatusOr<google::cloud::bigquery::v2::Job>(get_job_response.status()));
  }

  return JobPoll(*get_job_response, current_options, "InsertJob");
}

StreamRange<google::cloud::bigquery::v2::ListFormatJob>
ConnectionImpl::ListJobs(google::cloud::bigquery::v2::ListJobsRequest request,
                         Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  return job_connection_->ListJobs(request);
}

StatusOr<bigquery_unified::ReadArrowResponse> ConnectionImpl::ReadArrow(
    google::cloud::bigquery::storage::v1::CreateReadSessionRequest const&
        read_session_request,
    Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(
      internal::MergeOptions(std::move(opts), read_options_));
  auto current_options = google::cloud::internal::SaveCurrentOptions();

  auto session = read_connection_->CreateReadSession(read_session_request);
  if (!session) return std::move(session).status();

  bigquery_unified::ReadArrowResponse read_response;
  auto arrow_schema = GetArrowSchema(session->arrow_schema());
  if (!arrow_schema) return std::move(arrow_schema).status();
  read_response.estimated_total_bytes_scanned =
      session->estimated_total_bytes_scanned();
  read_response.estimated_total_physical_file_size =
      session->estimated_total_physical_file_size();
  read_response.estimated_row_count = session->estimated_row_count();
  read_response.expire_time = session->expire_time();
  read_response.schema = arrow_schema->first;

  for (auto const& s : session->streams()) {
    // It's important to call ReadRows from read_connection_ in order to
    // leverage the existing ResumableStreamingRead that it creates around
    // the call to ReadRows in its stub.
    auto factory = [connection = read_connection_,
                    current_options](google::cloud::bigquery::storage::v1::
                                         ReadRowsRequest const& r) mutable {
      google::cloud::internal::OptionsSpan span(*current_options);
      return std::make_shared<
          StreamRange<google::cloud::bigquery::storage::v1::ReadRowsResponse>>(
          connection->ReadRows(r));
    };

    auto arrow_stream_range = google::cloud::internal::MakeStreamRange<
        std::shared_ptr<arrow::RecordBatch>>(
        google::cloud::internal::MakeImmutableOptions(*current_options),
        ArrowRecordBatchReader(s.name(), arrow_schema->first,
                               arrow_schema->second, std::move(factory)));
    read_response.readers.push_back(std::move(arrow_stream_range));
  }

  return read_response;
}

Options ApplyUnifiedPolicyOptionsToJobServicePolicyOptions(Options options) {
  if (!options.has<bigquerycontrol_v2::JobServiceBackoffPolicyOption>()) {
    options.set<bigquerycontrol_v2::JobServiceBackoffPolicyOption>(
        options.get<bigquery_unified::BackoffPolicyOption>()->clone());
  }

  if (!options.has<bigquerycontrol_v2::JobServiceRetryPolicyOption>()) {
    // Dynamic casting is required here because of the current class hierarchy
    // present in RetryPolicy types found in google-cloud-cpp which prevent
    // handling them generically like BackoffPolicy types can be. If someday we
    // compress the RetryPolicy hierarchy, this code can be simplified similar
    // to how BackOffPolicy types are handled.
    auto const& unified_retry_policy =
        options.get<bigquery_unified::RetryPolicyOption>();
    if (auto* p = dynamic_cast<bigquery_unified::LimitedErrorCountRetryPolicy*>(
            unified_retry_policy.get());
        p != nullptr) {
      options.set<bigquerycontrol_v2::JobServiceRetryPolicyOption>(
          std::make_shared<
              bigquerycontrol_v2::JobServiceLimitedErrorCountRetryPolicy>(
              p->maximum_failures()));
    }
    if (auto* p = dynamic_cast<bigquery_unified::LimitedTimeRetryPolicy*>(
            unified_retry_policy.get());
        p != nullptr) {
      options.set<bigquerycontrol_v2::JobServiceRetryPolicyOption>(
          std::make_shared<
              bigquerycontrol_v2::JobServiceLimitedTimeRetryPolicy>(
              p->maximum_duration()));
    }
  }

  return options;
}

std::shared_ptr<bigquery_unified::Connection> MakeDefaultConnectionImpl(
    Options options) {
  internal::CheckExpectedOptions<
      CommonOptionList, GrpcOptionList, google::cloud::RestOptionList,
      UnifiedCredentialsOptionList,
      google::cloud::bigquery_unified::BigQueryJobOptionList,
      google::cloud::bigquery_unified::BigQueryReadOptionList,
      google::cloud::bigquerycontrol_v2::JobServicePolicyOptionList,
      google::cloud::bigquery_storage_v1::BigQueryReadPolicyOptionList>(
      options, __func__);

  auto background = std::make_unique<
      rest_internal::AutomaticallyCreatedRestBackgroundThreads>();

  options =
      ApplyUnifiedPolicyOptionsToJobServicePolicyOptions(std::move(options));

  auto read_options =
      bigquery_storage_v1_internal::BigQueryReadDefaultOptions(options);
  auto read_background =
      google::cloud::internal::MakeBackgroundThreadsFactory(read_options)();
  auto read_auth = google::cloud::internal::CreateAuthenticationStrategy(
      read_background->cq(), read_options);
  auto read_stub = google::cloud::bigquery_storage_v1_internal::
      CreateDefaultBigQueryReadStub(std::move(read_auth), read_options);
  auto read_connection =
      bigquery_storage_v1_internal::MakeBigQueryReadTracingConnection(
          std::make_shared<
              bigquery_storage_v1_internal::BigQueryReadConnectionImpl>(
              std::move(read_background), std::move(read_stub), read_options));

  auto job_options =
      bigquerycontrol_v2_internal::JobServiceDefaultOptions(options);
  // TODO: JobServiceRestConnectionImpl requires background threads, but does it
  // actually use them? Needs further investigation.
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
  return MakeTracingConnection(
      std::make_shared<bigquery_unified_internal::ConnectionImpl>(
          std::move(read_connection), std::move(job_connection),
          std::move(read_options), std::move(job_options), std::move(job_stub),
          std::move(background), std::move(options)));
}

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_internal
