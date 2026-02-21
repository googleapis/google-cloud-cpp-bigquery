// Copyright 2026 Google LLC
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

// This sample demonstrates how to execute a BigQuery query using the
// insert+poll workflow and how to read query results using futures
// and callbacks.

#include "google/cloud/bigquery_unified/client.h"

#include <future>
#include <iostream>
#include <string>

namespace bigquery = ::google::cloud::bigquery_unified;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0]
              << " <project-id> <dataset-id>\n";
    return 1;
  }

  std::string project_id = argv[1];
  std::string dataset_id = argv[2];

  // Create a BigQuery client using default credentials (creating connection with google cloud-secure and standard)
  auto client =
      bigquery::Client(bigquery::MakeBigQueryConnection());

  // A simple, read-only query suitable for samples.
  std::string query =
      "SELECT table_name FROM `" + project_id + "." + dataset_id +
      ".INFORMATION_SCHEMA.TABLES` LIMIT 10";

  // Execute the query using the insert+poll workflow.
  auto query_future = client.InsertQueryAndPoll(query);

  // Attach a continuation to process the results once the query completes.
  auto result_future = query_future.then(
      [](auto f) {
        auto response = f.get();
        if (!response) {
          std::cerr << "Query failed: "
                    << response.status() << "\n";
          return;
        }

        // Iterate over rows and fields in the result set.
        for (auto const& row : response->rows) {
          for (auto const& field : row) {
            std::cout << field.value << " ";
          }
          std::cout << "\n";
        }
      });

  // Wait for the callback to finish.
  result_future.get();

  return 0;
}
