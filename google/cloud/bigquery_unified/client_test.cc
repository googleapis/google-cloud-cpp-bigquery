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
#include "google/cloud/bigquery_unified/job_options.h"
#include "google/cloud/bigquery_unified/mocks/mock_connection.h"
#include "google/cloud/bigquery_unified/testing_util/status_matchers.h"
#include "google/cloud/internal/make_status.h"

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
namespace {

using ::google::cloud::bigquery_unified::testing_util::IsOkAndHolds;
using ::google::cloud::bigquery_unified::testing_util::StatusIs;
using ::google::cloud::bigquery_unified_mocks::MockConnection;
using ::testing::AllOf;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::IsSupersetOf;
using ::testing::Pair;
using ::testing::ResultOf;
using ::testing::Return;

struct TestOption {
  using Type = std::string;
};

TEST(BigQueryUnifiedClientTest, GetJobSuccess) {
  auto call_options = Options{}.set<TestOption>("call-test-option");

  auto mock_connection = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock_connection, options).WillRepeatedly(Return(Options{}));
  EXPECT_CALL(*mock_connection, GetJob)
      .WillOnce([&](google::cloud::bigquery::v2::GetJobRequest const& request,
                    Options opts) {
        EXPECT_THAT(request.project_id(), Eq("my-project-id"));
        EXPECT_THAT(opts.get<TestOption>(), Eq(call_options.get<TestOption>()));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id("my-project-id");
        job.mutable_job_reference()->set_job_id("my-job-id");
        return job;
      });

  auto client =
      Client(mock_connection, Options{}.set<TestOption>("client-test-option"));
  google::cloud::bigquery::v2::GetJobRequest request;
  request.set_project_id("my-project-id");
  auto get_result = client.GetJob(request, call_options);
  EXPECT_THAT(
      get_result,
      IsOkAndHolds(AllOf(
          ResultOf(
              "project id",
              [](auto const& x) { return x.job_reference().project_id(); },
              "my-project-id"),
          ResultOf(
              "job id",
              [](auto const& x) { return x.job_reference().job_id(); },
              "my-job-id"))));
}

TEST(BigQueryUnifiedClientTest, ReadArrowJobExtract) {
  std::string const project_id = "my-project";
  std::string const job_id = "my-job";
  std::string const job_type = "EXTRACT";

  auto mock_connection = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock_connection, options).WillRepeatedly(Return(Options{}));
  auto client = Client(mock_connection, Options{});
  google::cloud::bigquery::v2::Job job;
  job.mutable_job_reference()->set_project_id(project_id);
  job.mutable_job_reference()->set_job_id(job_id);
  job.mutable_configuration()->set_job_type(job_type);

  auto result = client.ReadArrow(job, {});
  EXPECT_THAT(
      result,
      StatusIs(
          StatusCode::kInvalidArgument,
          HasSubstr("Job: my-job is not a COPY, LOAD, or QUERY type job.")));
  EXPECT_THAT(result.status().error_info().metadata(),
              ::testing::IsSupersetOf({Pair("project_id", project_id),
                                       Pair("job_id", job_id),
                                       Pair("job_type", job_type)}));
}

TEST(BigQueryUnifiedClientTest, ReadArrowJobQuery) {
  std::string const billing_project_id = "my-billing_project";
  std::string const project_id = "my-project";
  std::string const job_id = "my-job";
  std::string const job_type = "QUERY";
  std::string const dataset_id = "my-dataset";
  std::string const table_id = "my-table";

  auto mock_connection = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock_connection, options).WillRepeatedly(Return(Options{}));
  EXPECT_CALL(*mock_connection, ReadArrow)
      .WillOnce([&](google::cloud::bigquery::storage::v1::
                        CreateReadSessionRequest const& request,
                    Options) -> StatusOr<ReadArrowResponse> {
        EXPECT_THAT(request.parent(), Eq("projects/" + billing_project_id));
        EXPECT_THAT(
            request.read_session().table(),
            Eq("projects/my-project/datasets/my-dataset/tables/my-table"));
        return internal::PermissionDeniedError("uh-oh");
      });

  auto client = Client(mock_connection,
                       Options{}.set<bigquery_unified::BillingProjectOption>(
                           billing_project_id));
  google::cloud::bigquery::v2::Job job;
  job.mutable_job_reference()->set_project_id(project_id);
  job.mutable_job_reference()->set_job_id(job_id);
  job.mutable_configuration()->set_job_type(job_type);
  auto* mutable_destination_table =
      job.mutable_configuration()->mutable_query()->mutable_destination_table();
  mutable_destination_table->set_project_id(project_id);
  mutable_destination_table->set_dataset_id(dataset_id);
  mutable_destination_table->set_table_id(table_id);

  auto result = client.ReadArrow(job, {});
  EXPECT_THAT(result, StatusIs(StatusCode::kPermissionDenied));
}

TEST(BigQueryUnifiedClientTest, ReadArrowJobCopy) {
  std::string const project_id = "my-project";
  std::string const job_id = "my-job";
  std::string const job_type = "COPY";
  std::string const dataset_id = "my-dataset";
  std::string const table_id = "my-table";

  auto mock_connection = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock_connection, options).WillRepeatedly(Return(Options{}));
  EXPECT_CALL(*mock_connection, ReadArrow)
      .WillOnce([&](google::cloud::bigquery::storage::v1::
                        CreateReadSessionRequest const& request,
                    Options) -> StatusOr<ReadArrowResponse> {
        EXPECT_THAT(request.parent(), Eq("projects/" + project_id));
        EXPECT_THAT(
            request.read_session().table(),
            Eq("projects/my-project/datasets/my-dataset/tables/my-table"));
        return internal::PermissionDeniedError("uh-oh");
      });

  auto client =
      Client(mock_connection, Options{}.set<TestOption>("client-test-option"));
  google::cloud::bigquery::v2::Job job;
  job.mutable_job_reference()->set_project_id(project_id);
  job.mutable_job_reference()->set_job_id(job_id);
  job.mutable_configuration()->set_job_type(job_type);
  auto* mutable_destination_table =
      job.mutable_configuration()->mutable_copy()->mutable_destination_table();
  mutable_destination_table->set_project_id(project_id);
  mutable_destination_table->set_dataset_id(dataset_id);
  mutable_destination_table->set_table_id(table_id);

  auto result = client.ReadArrow(job, {});
  EXPECT_THAT(result, StatusIs(StatusCode::kPermissionDenied));
}

TEST(BigQueryUnifiedClientTest, ReadArrowJobLoad) {
  std::string const project_id = "my-project";
  std::string const job_id = "my-job";
  std::string const job_type = "LOAD";
  std::string const dataset_id = "my-dataset";
  std::string const table_id = "my-table";

  auto mock_connection = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock_connection, options).WillRepeatedly(Return(Options{}));
  EXPECT_CALL(*mock_connection, ReadArrow)
      .WillOnce([&](google::cloud::bigquery::storage::v1::
                        CreateReadSessionRequest const& request,
                    Options) -> StatusOr<ReadArrowResponse> {
        EXPECT_THAT(request.parent(), Eq("projects/" + project_id));
        EXPECT_THAT(
            request.read_session().table(),
            Eq("projects/my-project/datasets/my-dataset/tables/my-table"));
        return internal::PermissionDeniedError("uh-oh");
      });

  auto client =
      Client(mock_connection, Options{}.set<TestOption>("client-test-option"));
  google::cloud::bigquery::v2::Job job;
  job.mutable_job_reference()->set_project_id(project_id);
  job.mutable_job_reference()->set_job_id(job_id);
  job.mutable_configuration()->set_job_type(job_type);
  auto* mutable_destination_table =
      job.mutable_configuration()->mutable_load()->mutable_destination_table();
  mutable_destination_table->set_project_id(project_id);
  mutable_destination_table->set_dataset_id(dataset_id);
  mutable_destination_table->set_table_id(table_id);

  auto result = client.ReadArrow(job, {});
  EXPECT_THAT(result, StatusIs(StatusCode::kPermissionDenied));
}

TEST(BigQueryUnifiedClientTest, ReadArrowJobReference) {
  std::string const project_id = "my-project";
  std::string const job_id = "my-job";
  std::string const job_type = "LOAD";
  std::string const dataset_id = "my-dataset";
  std::string const table_id = "my-table";

  auto mock_connection = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock_connection, options).WillRepeatedly(Return(Options{}));
  EXPECT_CALL(*mock_connection, GetJob)
      .WillOnce([&](google::cloud::bigquery::v2::GetJobRequest const& request,
                    Options) {
        EXPECT_THAT(request.project_id(), Eq(project_id));
        EXPECT_THAT(request.job_id(), Eq(job_id));

        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id(project_id);
        job.mutable_job_reference()->set_job_id(job_id);
        job.mutable_configuration()->set_job_type(job_type);
        auto* mutable_destination_table = job.mutable_configuration()
                                              ->mutable_load()
                                              ->mutable_destination_table();
        mutable_destination_table->set_project_id(project_id);
        mutable_destination_table->set_dataset_id(dataset_id);
        mutable_destination_table->set_table_id(table_id);
        return job;
      });

  EXPECT_CALL(*mock_connection, ReadArrow)
      .WillOnce([&](google::cloud::bigquery::storage::v1::
                        CreateReadSessionRequest const& request,
                    Options) -> StatusOr<ReadArrowResponse> {
        EXPECT_THAT(request.parent(), Eq("projects/" + project_id));
        EXPECT_THAT(
            request.read_session().table(),
            Eq("projects/my-project/datasets/my-dataset/tables/my-table"));
        return internal::PermissionDeniedError("uh-oh");
      });

  auto client = Client(mock_connection, Options{});
  google::cloud::bigquery::v2::JobReference job_reference;
  job_reference.set_project_id(project_id);
  job_reference.set_job_id(job_id);
  auto result = client.ReadArrow(job_reference, {});
  EXPECT_THAT(result, StatusIs(StatusCode::kPermissionDenied));
}

TEST(BigQueryUnifiedClientTest, ReadArrowJobTableReference) {
  std::string const project_id = "my-project";
  std::string const dataset_id = "my-dataset";
  std::string const table_id = "my-table";

  auto mock_connection = std::make_shared<MockConnection>();
  EXPECT_CALL(*mock_connection, options).WillRepeatedly(Return(Options{}));
  EXPECT_CALL(*mock_connection, ReadArrow)
      .WillOnce([&](google::cloud::bigquery::storage::v1::
                        CreateReadSessionRequest const& request,
                    Options) -> StatusOr<ReadArrowResponse> {
        EXPECT_THAT(request.parent(), Eq("projects/" + project_id));
        EXPECT_THAT(
            request.read_session().table(),
            Eq("projects/my-project/datasets/my-dataset/tables/my-table"));
        return internal::PermissionDeniedError("uh-oh");
      });

  auto client = Client(mock_connection, Options{});
  google::cloud::bigquery::v2::TableReference table_reference;
  table_reference.set_project_id(project_id);
  table_reference.set_dataset_id(dataset_id);
  table_reference.set_table_id(table_id);

  auto result = client.ReadArrow(table_reference, {});
  EXPECT_THAT(result, StatusIs(StatusCode::kPermissionDenied));
}

}  // namespace
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified
