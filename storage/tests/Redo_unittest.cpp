#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "glog/logging.h"

#include "catalog/CatalogAttribute.hpp"
#include "catalog/CatalogRelation.hpp"
#include "expressions/predicate/ComparisonPredicate.hpp"
#include "expressions/scalar/Scalar.hpp"
#include "expressions/scalar/ScalarAttribute.hpp"
#include "expressions/scalar/ScalarLiteral.hpp"
#include "log/Helper.hpp"
#include "log/LogManager.hpp"
#include "log/LogRecord.hpp"
#include "log/Macros.hpp"
#include "storage/InsertDestination.hpp"
#include "storage/StorageBlockLayout.hpp"
#include "storage/StorageManager.hpp"
#include "storage/ValueAccessor.hpp"
#include "storage/ValueAccessorUtil.hpp"
#include "tmb/id_typedefs.h"
#include "types/CharType.hpp"
#include "types/DoubleType.hpp"
#include "types/FloatType.hpp"
#include "types/IntType.hpp"
#include "types/LongType.hpp"
#include "types/TypeID.hpp"
#include "types/VarCharType.hpp"
#include "types/containers/Tuple.hpp"
#include "types/operations/comparisons/ComparisonFactory.hpp"
#include "types/operations/comparisons/ComparisonID.hpp"

namespace quickstep {

class RedoTest : public ::testing::Test {
 protected:
  static const relation_id kRelationId = 100;
  static const int kTupleNumber = 10;
  static const int kMaxAttributeId = 3;
  static const TransactionId tid = 5;

  virtual void SetUp() {
    log_manager_.reset(new LogManager());
    relation_.reset(new CatalogRelation(NULL, "TestRelation", kRelationId));
    storage_manager_.reset(new StorageManager("relation"));
    bus_.Initialize();
    foreman_client_id_ = bus_.Connect();

    // Add different kinds of attributes: int, double, char, varchar
    CatalogAttribute *int_attr = new CatalogAttribute(relation_.get(),
                                                      "int_attr",
                                                      TypeFactory::GetType(kInt, true));

    relation_->addAttribute(int_attr);

    CatalogAttribute *double_attr = new CatalogAttribute(relation_.get(),
                                                         "double_attr",
                                                         TypeFactory::GetType(kDouble, true));
    relation_->addAttribute(double_attr);

    CatalogAttribute *char_attr = new CatalogAttribute(relation_.get(),
                                                              "char_attr",
                                                              TypeFactory::GetType(kChar, 4, true));
    relation_->addAttribute(char_attr);

    CatalogAttribute *varchar_attr = new CatalogAttribute(relation_.get(),
                                                        "varchar_attr",
                                                        TypeFactory::GetType(kVarChar, 32, true));
    relation_->addAttribute(varchar_attr);
    
    // Records the 'base_value' of a tuple used in createSampleTuple.
    CatalogAttribute *base_value_attr = new CatalogAttribute(relation_.get(),
                                                             "base_value",
                                                             TypeFactory::GetType(kInt, false));
    relation_->addAttribute(base_value_attr);
    
    layout_.reset(StorageBlockLayout::GenerateDefaultLayout(*relation_, true));
    block_id_ = storage_manager_->createBlock(*relation_, layout_.get());
    relation_->addBlock(block_id_);
  }

  // Caller takes ownership of new heap-created Tuple.
  Tuple* createTuple(const int base_value) const {
    std::vector<TypedValue> attrs;

    // int_attr
    if (base_value % 8 == 0) {
      // Throw in a NULL integer for every eighth value.
      attrs.emplace_back(kInt);
    } else {
      attrs.emplace_back(base_value);
    }

    // double_attr
    if (base_value % 8 == 2) {
      // NULL very eighth value.
      attrs.emplace_back(kDouble);
    } else {
      attrs.emplace_back(static_cast<double>(0.25 * base_value));
    }

    // narrow_char_attr
    if (base_value % 8 == 4) {
      // NULL very eighth value.
      attrs.emplace_back(CharType::InstanceNullable(4).makeNullValue());
    } else {
      std::ostringstream char_buffer;
      char_buffer << base_value;
      std::string string_literal(char_buffer.str());
      attrs.emplace_back(CharType::InstanceNonNullable(4).makeValue(
          string_literal.c_str(),
          string_literal.size() > 3 ? 4
                                    : string_literal.size() + 1));
      attrs.back().ensureNotReference();
    }

    // wide_char_attr
    if (base_value % 8 == 6) {
      // NULL very eighth value.
      attrs.emplace_back(CharType::InstanceNullable(32).makeNullValue());
    } else {
      std::ostringstream char_buffer;
      char_buffer << "Here are some numbers: " << base_value;
      std::string string_literal(char_buffer.str());
      attrs.emplace_back(CharType::InstanceNonNullable(32).makeValue(
          string_literal.c_str(),
          string_literal.size() + 1));
      attrs.back().ensureNotReference();
    }

    // base_value
    attrs.emplace_back(base_value);
    return new Tuple(std::move(attrs));
  }

  // Insert some number of tuples into the block
  void insertSampleTuples(block_id bid, int num) {
    MutableBlockReference block = storage_manager_->getBlockMutable(bid, *relation_);
    
    for (int i = 0; i < num; ++i) {
      std::unique_ptr<Tuple> tuple(createTuple(i));
      block->insertTupleInBatch(*tuple);
    }
  }

  // update the base tuple to a series of new values
  // base_value will remain unchanged
  // new_value of 0 means NULL for all non-base fields
  inline StorageBlock::UpdateResult update(block_id bid, int base_value, int new_value) {
    DEBUG_ASSERT(base_value >= 0);
    DEBUG_ASSERT(base_value < kTupleNumber);

    MutableBlockReference block = storage_manager_->getBlockMutable(bid, *relation_);
    
    std::unique_ptr<std::unordered_map<attribute_id, std::unique_ptr<const Scalar>>> assignments;
    std::unique_ptr<Predicate> predicate;
    std::unique_ptr<InsertDestination> relocation;

    assignments.reset(new std::unordered_map<attribute_id, std::unique_ptr<const Scalar>>());
    if (new_value == 0) {
      assignments->emplace(0, std::make_unique<ScalarLiteral>(TypedValue(kInt),
                                                              TypeFactory::GetType(kInt, true)));
      assignments->emplace(1, std::make_unique<ScalarLiteral>(TypedValue(kDouble),
                                                              TypeFactory::GetType(kDouble, true)));
      assignments->emplace(2, std::make_unique<ScalarLiteral>(CharType::InstanceNullable(4).makeNullValue(),
                                                              TypeFactory::GetType(kChar, 4, true)));
      assignments->emplace(3, std::make_unique<ScalarLiteral>(VarCharType::InstanceNullable(32).makeNullValue(),
                                                              TypeFactory::GetType(kVarChar, 32, true)));
    }
    else {
      assignments->emplace(0, std::make_unique<ScalarLiteral>(TypedValue(new_value),
                                                              TypeFactory::GetType(kInt, true)));
      assignments->emplace(1, std::make_unique<ScalarLiteral>(TypedValue(static_cast<double> (0.25 * new_value)),
                                                              TypeFactory::GetType(kDouble, true)));

      std::ostringstream char_buffer;
      char_buffer << new_value;
      std::string char_string(char_buffer.str());
      assignments->emplace(2, std::make_unique<ScalarLiteral>(CharType::InstanceNonNullable(4).makeValue(
                                                              char_string.c_str(),
                                                              char_string.size() > 3 ? 4
                                                                : char_string.size() + 1),
                                                              TypeFactory::GetType(kChar, 4, true)));

      std::ostringstream varchar_buffer;
      varchar_buffer << "Here are some numbers: " << new_value;
      std::string varchar_string(varchar_buffer.str());
      assignments->emplace(3, std::make_unique<ScalarLiteral>(VarCharType::InstanceNonNullable(32).makeValue(
                                                                          varchar_string.c_str(),
                                                                          varchar_string.size() + 1),
                                                              TypeFactory::GetType(kVarChar, 32, true)));
    }

    predicate.reset(new ComparisonPredicate(ComparisonFactory::GetComparison(ComparisonID::kEqual),
                                            new ScalarAttribute(*(relation_->getAttributeByName("base_value"))),
                                            new ScalarLiteral(TypedValue(base_value),
                                                              TypeFactory::GetType(kInt, true))));

    relocation.reset(new BlockPoolInsertDestination(storage_manager_.get(),
                                                    relation_.get(),
                                                    NULL,
                                                    0,
                                                    foreman_client_id_,
                                                    &bus_));

    return block->update((*assignments.get()), predicate.get(), relocation.get(),
                        tid, log_manager_.get());
  }

  // Check if the two blocks are the same except block_id
  bool areTwoBlocksSame (block_id id1, block_id id2) {
    MutableBlockReference block1 = storage_manager_->getBlockMutable(id1, *relation_);
    MutableBlockReference block2 = storage_manager_->getBlockMutable(id2, *relation_);

    std::unique_ptr<ValueAccessor> accessor1(block1->getTupleStorageSubBlock().createValueAccessor());    
    std::unique_ptr<ValueAccessor> accessor2(block2->getTupleStorageSubBlock().createValueAccessor());
    return InvokeOnAnyValueAccessor (
        accessor1.get(),
        [&] (auto *accessor1) -> bool {
      accessor1->beginIteration();

      return InvokeOnAnyValueAccessor (
          accessor2.get(),
          [&] (auto *accessor2) -> bool {

        accessor2->beginIteration();
        while (!accessor1->next() && !accessor2->next()) {
          Tuple* tuple1 = accessor1->getTuple();
          Tuple* tuple2 = accessor2->getTuple();
          Tuple::const_iterator it1, it2;
          for (it1 = tuple1->begin(), it2 = tuple2->begin();
              it1 != tuple1->end() && it2 != tuple2->end();
              it1++, it2++) {
            bool equal = Helper::valueEqual(*it1, *it2);
            EXPECT_TRUE(equal);
            if (!equal) {
              return false;
            }
          }
        }

        return true;
      });
    });
  }
  
  std::unique_ptr<StorageManager> storage_manager_;
  block_id block_id_;
  std::unique_ptr<LogManager> log_manager_;
  std::unique_ptr<CatalogRelation> relation_;
  std::unique_ptr<StorageBlockLayout> layout_;
  MessageBusImpl bus_;
  tmb::client_id foreman_client_id_;
}; // class RedoTest

// Test if in-place update could be redo
TEST_F(RedoTest, InPlaceUpdateTest) {
  StorageBlock::UpdateResult result;
  // Rebuild the block in a new block in order to compare with the old one
  block_id new_block_id = storage_manager_->createBlock(*relation_, layout_.get());
  ASSERT_EQ(block_id_ + 1, new_block_id);

  // Insert tuples to both blocks
  insertSampleTuples(block_id_, kTupleNumber);
  insertSampleTuples(new_block_id, kTupleNumber);

  // Update tuples in the original block
  // Update a tuple without NULL value involved
  result = update(block_id_, 1, 5);
  EXPECT_TRUE(result.indices_consistent);
  EXPECT_FALSE(result.relocation_destination_used);
  // Update fields to NULL
  result = update(block_id_, 3, 0);
  EXPECT_TRUE(result.indices_consistent);
  EXPECT_FALSE(result.relocation_destination_used);
  // Update fields from Null
  result = update(block_id_, 0, 1);
  EXPECT_TRUE(result.indices_consistent);
  EXPECT_FALSE(result.relocation_destination_used);

  // Rebuild the block in block_id_+1 from log
  std::string buffer(log_manager_->getBuffer());
  while (buffer.length() > 0) {
    int length, index, payload_length;
    std::string header, payload;
    block_id bid;
    tuple_id tup_id;
    MutableBlockReference new_block;
    std::unique_ptr<std::unordered_map<attribute_id, TypedValue>> updated_values;
    
    // Get the log string from the buffer
    length = Helper::strToInt(buffer.substr(Macros::kLENGTH_START, sizeof(int)));
    payload_length = length - Macros::kHEADER_LENGTH;
    header = buffer.substr(0, Macros::kHEADER_LENGTH);
    payload = buffer.substr(Macros::kPAYLOAD_START, payload_length);
    buffer = buffer.substr(length);

    // Check type and block
    ASSERT_EQ((int) LogRecord::LogRecordType::kUPDATE,
              (int) header.at(Macros::kTYPE_START));
    index = 0;
    bid = Helper::strToId(payload.substr(index, sizeof(block_id)));
    index += sizeof(block_id);
    ASSERT_EQ(block_id_, bid);
    new_block = storage_manager_->getBlockMutable(bid + 1, *relation_);

    // Redo the update
    tup_id = Helper::strToInt(payload.substr(index, sizeof(tuple_id)));
    index += sizeof(tuple_id);
    updated_values.reset(new std::unordered_map<attribute_id, TypedValue>());

    // For each attribute, find the updated value
    while (index < payload_length) {
      attribute_id attr_id = Helper::strToInt(payload.substr(index, sizeof(attribute_id)));
      index += sizeof(attribute_id);
      TypedValue pre_value = Helper::strToValue(payload.substr(index));
      index += Helper::valueLength(pre_value);
      TypedValue post_value = Helper::strToValue(payload.substr(index));
      index += Helper::valueLength(post_value);
      updated_values->insert(std::make_pair(attr_id, post_value));
    }
    
    new_block->updateTupleInPlace(updated_values.get(), tup_id);

    EXPECT_TRUE(areTwoBlocksSame(block_id_, new_block_id));

  }
}

} // namespace quickstep
