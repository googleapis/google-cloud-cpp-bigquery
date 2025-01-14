// Copyright 2024 Google LLC
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

#include "google/cloud/bigquery_unified/client.h"
#include "google/cloud/bigquery_unified/testing_util/status_matchers.h"
#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/internal/getenv.h"
#include <gmock/gmock.h>

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
namespace {

using ::testing::Eq;

class JobIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    project_id_ =
        google::cloud::internal::GetEnv("GOOGLE_CLOUD_PROJECT").value_or("");
    ASSERT_FALSE(project_id_.empty());
  }
  std::string project_id_;
};

TEST_F(JobIntegrationTest, GetJob) {
  namespace bigquery_proto = google::cloud::bigquery::v2;
  std::shared_ptr<Connection> connection =
      google::cloud::bigquery_unified::MakeConnection();
  auto client = google::cloud::bigquery_unified::Client(connection);

  // TODO: hard coding this id is brittle but currently necessary.
  std::string const job_id = "bquxjob_606db6e8_19465e708c7";
  bigquery_proto::GetJobRequest get_request;
  get_request.set_project_id(project_id_);
  get_request.set_job_id(job_id);
  auto job = client.GetJob(get_request);
  ASSERT_STATUS_OK(job);
  EXPECT_THAT(job->status().state(), Eq("DONE"));
}

TEST_F(JobIntegrationTest, DeleteJob) {
  namespace bigquery_proto = google::cloud::bigquery::v2;
  std::shared_ptr<Connection> connection =
      google::cloud::bigquery_unified::MakeConnection();
  auto client = google::cloud::bigquery_unified::Client(connection);

  // TODO: hard coding this id is brittle but currently necessary.
  std::string const job_id = "bquxjob_606db6e8_19465e708c7";

  bigquery_proto::DeleteJobRequest delete_request;
  delete_request.set_project_id(project_id_);
  delete_request.set_job_id(job_id);
  auto status = client.DeleteJob(delete_request);
  ASSERT_STATUS_OK(status);
}

}  // namespace
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified
