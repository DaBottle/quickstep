#   Copyright 2011-2015 Quickstep Technologies LLC.
#   Copyright 2015 Pivotal Software, Inc.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

add_test(quickstep_queryoptimizer_tests_physicalgenerator_copy
         "../quickstep_queryoptimizer_tests_OptimizerTextTest"
         "${CMAKE_CURRENT_SOURCE_DIR}/Copy.test"
         "${CMAKE_CURRENT_BINARY_DIR}/Copy.test")
add_test(quickstep_queryoptimizer_tests_physicalgenerator_create
         "../quickstep_queryoptimizer_tests_OptimizerTextTest"
         "${CMAKE_CURRENT_SOURCE_DIR}/Create.test"
         "${CMAKE_CURRENT_BINARY_DIR}/Create.test")
add_test(quickstep_queryoptimizer_tests_physicalgenerator_delete
         "../quickstep_queryoptimizer_tests_OptimizerTextTest"
         "${CMAKE_CURRENT_SOURCE_DIR}/Delete.test"
         "${CMAKE_CURRENT_BINARY_DIR}/Delete.test")
add_test(quickstep_queryoptimizer_tests_physicalgenerator_drop
         "../quickstep_queryoptimizer_tests_OptimizerTextTest"
         "${CMAKE_CURRENT_SOURCE_DIR}/Drop.test"
         "${CMAKE_CURRENT_BINARY_DIR}/Drop.test")
add_test(quickstep_queryoptimizer_tests_physicalgenerator_insert
         "../quickstep_queryoptimizer_tests_OptimizerTextTest"
         "${CMAKE_CURRENT_SOURCE_DIR}/Insert.test"
         "${CMAKE_CURRENT_BINARY_DIR}/Insert.test")
add_test(quickstep_queryoptimizer_tests_physicalgenerator_select
         "../quickstep_queryoptimizer_tests_OptimizerTextTest"
         "${CMAKE_CURRENT_SOURCE_DIR}/Select.test"
         "${CMAKE_CURRENT_BINARY_DIR}/Select.test")
add_test(quickstep_queryoptimizer_tests_physicalgenerator_update
         "../quickstep_queryoptimizer_tests_OptimizerTextTest"
         "${CMAKE_CURRENT_SOURCE_DIR}/Update.test"
         "${CMAKE_CURRENT_BINARY_DIR}/Update.test")
