add_library(quickstep_log_LogManager
            LogManager.cpp
            LogManager.hpp)

add_library(quickstep_log_LogRecord
            LogRecord.cpp
            LogRecord.hpp)

add_library(quickstep_log_LogTable
            LogTable.cpp
            LogTable.hpp)

add_library(quickstep_log_Helper
            Helper.cpp
            Helper.hpp)

add_library(quickstep_log_Macros
            Macros.hpp
            Macros.cpp)

target_link_libraries(quickstep_log_LogManager
                      quickstep_log_LogRecord
                      quickstep_log_LogTable
                      quickstep_log_Helper
                      quickstep_log_Macros
                      quickstep_utility_ThreadSafeQueue)

target_link_libraries(quickstep_log_LogRecord
                      quickstep_catalog_CatalogTypedefs
                      quickstep_storage_StorageBlockInfo
                      quickstep_storage_ValueAccessor
                      quickstep_log_Helper
                      quickstep_log_Macros
                      quickstep_transaction_Transaction)

target_link_libraries(quickstep_log_LogTable
                      quickstep_log_Helper
                      quickstep_transaction_Transaction)

target_link_libraries(quickstep_log_Helper
                      quickstep_transaction_Transaction
                      quickstep_types_operations_comparisons_EqualComparison
                      quickstep_types_TypedValue
                      quickstep_types_TypeFactory)

add_executable(LogManager_unittest
              "${CMAKE_CURRENT_SOURCE_DIR}/tests/LogManager_unittest.cpp")

target_link_libraries(LogManager_unittest
                      gtest
	                  gtest_main
                      quickstep_log_Helper
                      quickstep_log_LogManager
                      quickstep_log_LogRecord
                      quickstep_log_Macros
                      quickstep_storage_ValueAccessor
                      quickstep_storage_ValueAccessorUtil
                      quickstep_transaction_Transaction
                      quickstep_types_containers_Tuple
                      quickstep_types_operations_comparisons_EqualComparison
                      quickstep_types_TypeFactory
                      quickstep_types_TypeID)

add_executable(LogTable_unittest
              "${CMAKE_CURRENT_SOURCE_DIR}/tests/LogTable_unittest.cpp")

target_link_libraries(LogTable_unittest
                      gtest
	                    gtest_main
                      quickstep_log_LogManager
                      quickstep_transaction_Transaction)

add_executable(Helper_unittest
              "${CMAKE_CURRENT_SOURCE_DIR}/tests/Helper_unittest.cpp")

target_link_libraries(Helper_unittest
                      gtest
                      gtest_main
                      quickstep_log_Helper
                      quickstep_types_operations_comparisons_EqualComparison
                      quickstep_types_Type
                      quickstep_types_TypedValue
                      quickstep_types_TypeFactory
                      quickstep_types_TypeID)
