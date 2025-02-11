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

#ifndef GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_MOCKS_MOCK_CONNECTION_H
#define GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_MOCKS_MOCK_CONNECTION_H

#include "google/cloud/bigquery_unified/connection.h"
#include <gmock/gmock.h>

namespace google::cloud::bigquery_unified_mocks {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

/**
 * A class to mock `Connection`.
 *
 * Application developers may want to test their code with simulated responses,
 * including errors, from an object of type `Client`. To do so,
 * construct an object of type `Client` with an instance of this
 * class. Then use the Google Test framework functions to program the behavior
 * of this mock.
 */
class MockConnection : public bigquery_unified::Connection {
 public:
  MOCK_METHOD(Options, options, (), (override));

  // CancelJob
  MOCK_METHOD(future<StatusOr<google::cloud::bigquery::v2::Job>>, CancelJob,
              (google::cloud::bigquery::v2::CancelJobRequest const& request,
               Options opts),
              (override));

  MOCK_METHOD(StatusOr<google::cloud::bigquery::v2::JobReference>, CancelJob,
              (google::cloud::NoAwaitTag,
               google::cloud::bigquery::v2::CancelJobRequest const& request,
               Options opts),
              (override));

  MOCK_METHOD(future<StatusOr<google::cloud::bigquery::v2::Job>>, CancelJob,
              (google::cloud::bigquery::v2::JobReference const& job_reference,
               Options opts),
              (override));

  // GetJob
  MOCK_METHOD(StatusOr<google::cloud::bigquery::v2::Job>, GetJob,
              (google::cloud::bigquery::v2::GetJobRequest const& request,
               Options opts),
              (override));

  // DeleteJob
  MOCK_METHOD(Status, DeleteJob,
              (google::cloud::bigquery::v2::DeleteJobRequest const& request,
               Options opts),
              (override));

  // ListJobs
  MOCK_METHOD(StreamRange<google::cloud::bigquery::v2::ListFormatJob>, ListJobs,
              (google::cloud::bigquery::v2::ListJobsRequest request,
               Options opts),
              (override));

  // InsertJob
  MOCK_METHOD(future<StatusOr<google::cloud::bigquery::v2::Job>>, InsertJob,
              (google::cloud::bigquery::v2::Job const& job, Options opts),
              (override));

  MOCK_METHOD(StatusOr<google::cloud::bigquery::v2::JobReference>, InsertJob,
              (google::cloud::NoAwaitTag,
               google::cloud::bigquery::v2::Job const& job, Options opts),
              (override));

  MOCK_METHOD(future<StatusOr<google::cloud::bigquery::v2::Job>>, InsertJob,
              (google::cloud::bigquery::v2::JobReference const& job_reference,
               Options opts),
              (override));

  MOCK_METHOD(
      StatusOr<bigquery_unified::ReadArrowResponse>, ReadArrow,
      (google::cloud::bigquery::storage::v1::CreateReadSessionRequest const&
           read_session,
       Options opts),
      (override));
};

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_mocks

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_MOCKS_MOCK_CONNECTION_H
