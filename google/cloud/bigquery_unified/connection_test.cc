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

#include "google/cloud/bigquery_unified/connection.h"
#include "google/cloud/bigquery_unified/job_options.h"
#include "google/cloud/bigquery_unified/testing_util/opentelemetry_matchers.h"
#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/common_options.h"

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
namespace {

#ifdef GOOGLE_CLOUD_CPP_BIGQUERY_HAVE_OPENTELEMETRY
using ::google::cloud::bigquery_unified::testing_util::DisableTracing;
using ::google::cloud::bigquery_unified::testing_util::EnableTracing;
using ::google::cloud::bigquery_unified::testing_util::SpanNamed;
using ::testing::Contains;
using ::testing::Not;

TEST(BigQueryUnifiedConnectionTest, TracingEnabled) {
  auto span_catcher = testing_util::InstallSpanCatcher();

  auto options = EnableTracing(
      Options{}
          .set<EndpointOption>("localhost:1")
          .set<bigquery_unified::RetryPolicyOption>(
              std::make_shared<bigquery_unified::LimitedErrorCountRetryPolicy>(
                  0)));
  auto conn = MakeConnection(std::move(options));
  google::cloud::internal::OptionsSpan span(
      google::cloud::internal::MergeOptions(Options{}, conn->options()));
  // Make a call, which should fail fast. The error itself is not important.
  google::cloud::bigquery::v2::GetJobRequest request;
  (void)conn->GetJob(request, Options{});

  auto spans = span_catcher->GetSpans();
  EXPECT_THAT(spans,
              Contains(SpanNamed("bigquery_unified::Connection::GetJob")));
}

TEST(BigQueryUnifiedConnectionTest, TracingDisabled) {
  auto span_catcher = testing_util::InstallSpanCatcher();

  auto options = DisableTracing(
      Options{}
          .set<EndpointOption>("localhost:1")
          .set<bigquery_unified::RetryPolicyOption>(
              std::make_shared<bigquery_unified::LimitedErrorCountRetryPolicy>(
                  0)));
  auto conn = MakeConnection(std::move(options));
  google::cloud::internal::OptionsSpan span(
      google::cloud::internal::MergeOptions(Options{}, conn->options()));
  // Make a call, which should fail fast. The error itself is not important.
  google::cloud::bigquery::v2::GetJobRequest request;
  (void)conn->GetJob(request, Options{});

  auto spans = span_catcher->GetSpans();
  EXPECT_THAT(spans,
              Not(Contains(SpanNamed("bigquery_unified::Connection::GetJob"))));
}

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_HAVE_OPENTELEMETRY

}  // namespace
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified
