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

#ifndef GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_JOB_OPTIONS_H
#define GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_JOB_OPTIONS_H

#include "google/cloud/bigquery_unified/idempotency_policy.h"
#include "google/cloud/bigquery_unified/retry_policy.h"
#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/backoff_policy.h"
#include "google/cloud/options.h"
#include "google/cloud/polling_policy.h"
#include <memory>

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

/**
 * @defgroup google-cloud-bigquery-unified-options BigQuery Unified Options
 * @{
 */

/**
 * Use with `google::cloud::Options` to configure the backoff policy.
 *
 * @ingroup google-cloud-bigquery-unified-options
 */
struct BackoffPolicyOption {
  using Type = std::shared_ptr<BackoffPolicy>;
};

/**
 * Use with `google::cloud::Options` to specify the billing project.
 *
 * @ingroup google-cloud-bigquery-unified-options
 */
struct BillingProjectOption {
  using Type = std::string;
};

/**
 * Use with `google::cloud::Options` to configure which operations are retried.
 *
 * @ingroup google-cloud-bigquery-unified-options
 */
struct IdempotencyPolicyOption {
  using Type = std::shared_ptr<bigquery_unified::IdempotencyPolicy>;
};

/**
 * Use with `google::cloud::Options` to configure the polling policy.
 *
 * @ingroup google-cloud-bigquery-unified-options
 */
struct PollingPolicyOption {
  using Type = std::shared_ptr<PollingPolicy>;
};

/**
 * Use with `google::cloud::Options` to configure the retry policy.
 *
 * @ingroup google-cloud-bigquery-unified-options
 */
struct RetryPolicyOption {
  using Type = std::shared_ptr<bigquery_unified::RetryPolicy>;
};

/** @} */

using BigQueryJobOptionList =
    OptionList<BackoffPolicyOption, BillingProjectOption,
               IdempotencyPolicyOption, PollingPolicyOption, RetryPolicyOption>;

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_JOB_OPTIONS_H
