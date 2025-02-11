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
#include "google/cloud/bigquery_unified/mocks/mock_connection.h"
#include "google/cloud/bigquery_unified/mocks/mock_stream_range.h"
#include "google/cloud/bigquery_unified/testing_util/opentelemetry_matchers.h"
#include "google/cloud/bigquery_unified/testing_util/status_matchers.h"
#include "google/cloud/internal/make_status.h"
#include <gmock/gmock.h>
#include <memory>

namespace google::cloud::bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
namespace {

#ifdef GOOGLE_CLOUD_CPP_BIGQUERY_HAVE_OPENTELEMETRY
using ::google::cloud::bigquery_unified::testing_util::DisableTracing;
using ::google::cloud::bigquery_unified::testing_util::EnableTracing;
using ::google::cloud::bigquery_unified::testing_util::InstallSpanCatcher;
using ::google::cloud::bigquery_unified::testing_util::OTelAttribute;
using ::google::cloud::bigquery_unified::testing_util::OTelContextCaptured;
using ::google::cloud::bigquery_unified::testing_util::SpanHasAttributes;
using ::google::cloud::bigquery_unified::testing_util::
    SpanHasInstrumentationScope;
using ::google::cloud::bigquery_unified::testing_util::SpanKindIsClient;
using ::google::cloud::bigquery_unified::testing_util::SpanNamed;
using ::google::cloud::bigquery_unified::testing_util::SpanWithStatus;
using ::google::cloud::bigquery_unified::testing_util::StatusIs;
using ::google::cloud::bigquery_unified::testing_util::ThereIsAnActiveSpan;
using ::google::cloud::bigquery_unified_mocks::MockConnection;
using ::testing::_;
using ::testing::A;
using ::testing::AllOf;
using ::testing::ByMove;
using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::Not;
using ::testing::Return;

auto constexpr kErrorCode = "ABORTED";

TEST(TracingConnectionTest, CancelJobAwait) {
  auto span_catcher = InstallSpanCatcher();

  auto mock = std::make_shared<MockConnection>();
  EXPECT_CALL(
      *mock,
      CancelJob(A<google::cloud::bigquery::v2::CancelJobRequest const&>(),
                A<Options>()))
      .WillOnce([] {
        EXPECT_TRUE(ThereIsAnActiveSpan());
        EXPECT_TRUE(OTelContextCaptured());
        return make_ready_future<StatusOr<google::cloud::bigquery::v2::Job>>(
            internal::AbortedError("fail"));
      });

  auto under_test = TracingConnection(mock);
  google::cloud::bigquery::v2::CancelJobRequest request;
  auto result = under_test.CancelJob(request, Options{}).get();
  EXPECT_THAT(result, StatusIs(StatusCode::kAborted));

  auto spans = span_catcher->GetSpans();
  EXPECT_THAT(
      spans,
      ElementsAre(AllOf(
          SpanHasInstrumentationScope(), SpanKindIsClient(),
          SpanNamed("bigquery_unified::Connection::CancelJob"),
          SpanWithStatus(opentelemetry::trace::StatusCode::kError, "fail"),
          SpanHasAttributes(
              OTelAttribute<std::string>("gl-cpp.status_code", kErrorCode)))));
}

TEST(TracingConnectionTest, CancelJobNoAwait) {
  auto span_catcher = InstallSpanCatcher();

  auto mock = std::make_shared<MockConnection>();
  EXPECT_CALL(
      *mock,
      CancelJob(A<google::cloud::NoAwaitTag>(),
                A<google::cloud::bigquery::v2::CancelJobRequest const&>(),
                A<Options>()))
      .WillOnce([] {
        EXPECT_TRUE(ThereIsAnActiveSpan());
        return internal::AbortedError("fail");
      });

  auto under_test = TracingConnection(mock);
  google::cloud::bigquery::v2::CancelJobRequest request;
  auto result =
      under_test.CancelJob(google::cloud::NoAwaitTag{}, request, Options{});
  EXPECT_THAT(result, StatusIs(StatusCode::kAborted));

  auto spans = span_catcher->GetSpans();
  EXPECT_THAT(
      spans,
      ElementsAre(AllOf(
          SpanHasInstrumentationScope(), SpanKindIsClient(),
          SpanNamed("bigquery_unified::Connection::CancelJob"),
          SpanWithStatus(opentelemetry::trace::StatusCode::kError, "fail"),
          SpanHasAttributes(
              OTelAttribute<std::string>("gl-cpp.status_code", kErrorCode)))));
}

TEST(TracingConnectionTest, CancelJobPoll) {
  auto span_catcher = InstallSpanCatcher();

  auto mock = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock,
              CancelJob(A<google::cloud::bigquery::v2::JobReference const&>(),
                        A<Options>()))
      .WillOnce([] {
        EXPECT_TRUE(ThereIsAnActiveSpan());
        EXPECT_TRUE(OTelContextCaptured());
        return make_ready_future<StatusOr<google::cloud::bigquery::v2::Job>>(
            internal::AbortedError("fail"));
      });

  auto under_test = TracingConnection(mock);
  google::cloud::bigquery::v2::JobReference jobref;
  auto result = under_test.CancelJob(jobref, Options{}).get();
  EXPECT_THAT(result, StatusIs(StatusCode::kAborted));

  auto spans = span_catcher->GetSpans();
  EXPECT_THAT(
      spans,
      ElementsAre(AllOf(
          SpanHasInstrumentationScope(), SpanKindIsClient(),
          SpanNamed("bigquery_unified::Connection::CancelJob"),
          SpanWithStatus(opentelemetry::trace::StatusCode::kError, "fail"),
          SpanHasAttributes(
              OTelAttribute<std::string>("gl-cpp.status_code", kErrorCode)))));
}

TEST(TracingConnectionTest, GetJob) {
  auto span_catcher = InstallSpanCatcher();

  auto mock = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock, GetJob).WillOnce([] {
    EXPECT_TRUE(ThereIsAnActiveSpan());
    return internal::AbortedError("fail");
  });

  auto under_test = TracingConnection(mock);
  google::cloud::bigquery::v2::GetJobRequest request;
  auto result = under_test.GetJob(request, Options{});
  EXPECT_THAT(result, StatusIs(StatusCode::kAborted));

  auto spans = span_catcher->GetSpans();
  EXPECT_THAT(
      spans,
      ElementsAre(AllOf(
          SpanHasInstrumentationScope(), SpanKindIsClient(),
          SpanNamed("bigquery_unified::Connection::GetJob"),
          SpanWithStatus(opentelemetry::trace::StatusCode::kError, "fail"),
          SpanHasAttributes(
              OTelAttribute<std::string>("gl-cpp.status_code", kErrorCode)))));
}

TEST(TracingConnectionTest, InsertJobAwait) {
  auto span_catcher = InstallSpanCatcher();

  auto mock = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock, InsertJob(A<google::cloud::bigquery::v2::Job const&>(),
                               A<Options>()))
      .WillOnce([] {
        EXPECT_TRUE(ThereIsAnActiveSpan());
        EXPECT_TRUE(OTelContextCaptured());
        return make_ready_future<StatusOr<google::cloud::bigquery::v2::Job>>(
            internal::AbortedError("fail"));
      });

  auto under_test = TracingConnection(mock);
  google::cloud::bigquery::v2::Job job;
  auto result = under_test.InsertJob(job, Options{}).get();
  EXPECT_THAT(result, StatusIs(StatusCode::kAborted));

  auto spans = span_catcher->GetSpans();
  EXPECT_THAT(
      spans,
      ElementsAre(AllOf(
          SpanHasInstrumentationScope(), SpanKindIsClient(),
          SpanNamed("bigquery_unified::Connection::InsertJob"),
          SpanWithStatus(opentelemetry::trace::StatusCode::kError, "fail"),
          SpanHasAttributes(
              OTelAttribute<std::string>("gl-cpp.status_code", kErrorCode)))));
}

TEST(TracingConnectionTest, InsertJobNoAwait) {
  auto span_catcher = InstallSpanCatcher();

  auto mock = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock, InsertJob(A<google::cloud::NoAwaitTag>(),
                               A<google::cloud::bigquery::v2::Job const&>(),
                               A<Options>()))
      .WillOnce([] {
        EXPECT_TRUE(ThereIsAnActiveSpan());
        return internal::AbortedError("fail");
      });

  auto under_test = TracingConnection(mock);
  google::cloud::bigquery::v2::Job job;
  auto result =
      under_test.InsertJob(google::cloud::NoAwaitTag{}, job, Options{});
  EXPECT_THAT(result, StatusIs(StatusCode::kAborted));

  auto spans = span_catcher->GetSpans();
  EXPECT_THAT(
      spans,
      ElementsAre(AllOf(
          SpanHasInstrumentationScope(), SpanKindIsClient(),
          SpanNamed("bigquery_unified::Connection::InsertJob"),
          SpanWithStatus(opentelemetry::trace::StatusCode::kError, "fail"),
          SpanHasAttributes(
              OTelAttribute<std::string>("gl-cpp.status_code", kErrorCode)))));
}

TEST(TracingConnectionTest, InsertJobPoll) {
  auto span_catcher = InstallSpanCatcher();

  auto mock = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock,
              InsertJob(A<google::cloud::bigquery::v2::JobReference const&>(),
                        A<Options>()))
      .WillOnce([] {
        EXPECT_TRUE(ThereIsAnActiveSpan());
        EXPECT_TRUE(OTelContextCaptured());
        return make_ready_future<StatusOr<google::cloud::bigquery::v2::Job>>(
            internal::AbortedError("fail"));
      });

  auto under_test = TracingConnection(mock);
  google::cloud::bigquery::v2::JobReference jobref;
  auto result = under_test.InsertJob(jobref, Options{}).get();
  EXPECT_THAT(result, StatusIs(StatusCode::kAborted));

  auto spans = span_catcher->GetSpans();
  EXPECT_THAT(
      spans,
      ElementsAre(AllOf(
          SpanHasInstrumentationScope(), SpanKindIsClient(),
          SpanNamed("bigquery_unified::Connection::InsertJob"),
          SpanWithStatus(opentelemetry::trace::StatusCode::kError, "fail"),
          SpanHasAttributes(
              OTelAttribute<std::string>("gl-cpp.status_code", kErrorCode)))));
}

TEST(TracingConnectionTest, DeleteJob) {
  auto span_catcher = InstallSpanCatcher();

  auto mock = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock, DeleteJob).WillOnce([] {
    EXPECT_TRUE(ThereIsAnActiveSpan());
    return internal::AbortedError("fail");
  });

  auto under_test = TracingConnection(mock);
  google::cloud::bigquery::v2::DeleteJobRequest request;
  auto result = under_test.DeleteJob(request, Options{});
  EXPECT_THAT(result, StatusIs(StatusCode::kAborted));

  auto spans = span_catcher->GetSpans();
  EXPECT_THAT(
      spans,
      ElementsAre(AllOf(
          SpanHasInstrumentationScope(), SpanKindIsClient(),
          SpanNamed("bigquery_unified::Connection::DeleteJob"),
          SpanWithStatus(opentelemetry::trace::StatusCode::kError, "fail"),
          SpanHasAttributes(
              OTelAttribute<std::string>("gl-cpp.status_code", kErrorCode)))));
}

TEST(TracingConnectionTest, ListJobs) {
  auto span_catcher = InstallSpanCatcher();

  auto mock = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock, ListJobs).WillOnce([] {
    EXPECT_TRUE(ThereIsAnActiveSpan());
    EXPECT_TRUE(OTelContextCaptured());
    return bigquery_unified_mocks::MakeStreamRange<
        google::cloud::bigquery::v2::ListFormatJob>(
        {}, internal::AbortedError("fail"));
  });

  auto under_test = TracingConnection(mock);
  google::cloud::bigquery::v2::ListJobsRequest request;
  auto stream = under_test.ListJobs(request, Options{});
  auto it = stream.begin();
  EXPECT_THAT(*it, StatusIs(StatusCode::kAborted));
  EXPECT_EQ(++it, stream.end());

  auto spans = span_catcher->GetSpans();
  EXPECT_THAT(
      spans,
      ElementsAre(AllOf(
          SpanHasInstrumentationScope(), SpanKindIsClient(),
          SpanNamed("bigquery_unified::Connection::ListJobs"),
          SpanWithStatus(opentelemetry::trace::StatusCode::kError, "fail"),
          SpanHasAttributes(
              OTelAttribute<std::string>("gl-cpp.status_code", kErrorCode)))));
}

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_HAVE_OPENTELEMETRY

}  // namespace
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_internal
