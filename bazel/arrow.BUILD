# Copyright 2024 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

package(default_visibility = ["//visibility:public"])

# Arrow dependency. Our CI builds expect the libraries to be in
# /usr/local/lib64.
# You may need to create a symbolic link on other systems, e.g.:
# ln -s /usr/local/lib /usr/local/lib64
cc_import(
    name = "libarrow",
    hdrs = glob(["include/arrow/*.h"]),
    shared_library = "lib64/libarrow.so.1801",
    static_library = "lib64/libarrow.a",
    visibility = ["//visibility:public"],
)
