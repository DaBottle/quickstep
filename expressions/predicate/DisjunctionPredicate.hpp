/**
 * This file copyright (c) 2011-2015, Quickstep Technologies LLC.
 * Copyright (c) 2015, Pivotal Software, Inc.
 * All rights reserved.
 * See file CREDITS.txt for details.
 **/

#ifndef QUICKSTEP_EXPRESSIONS_PREDICATE_DISJUNCTION_PREDICATE_HPP_
#define QUICKSTEP_EXPRESSIONS_PREDICATE_DISJUNCTION_PREDICATE_HPP_

#include "catalog/CatalogTypedefs.hpp"
#include "expressions/Expressions.pb.h"
#include "expressions/predicate/Predicate.hpp"
#include "expressions/predicate/PredicateWithList.hpp"
#include "storage/StorageBlockInfo.hpp"
#include "utility/Macros.hpp"

namespace quickstep {

class TupleIdSequence;
class ValueAccessor;

struct SubBlocksReference;

/** \addtogroup Expressions
 *  @{
 */

/**
 * @brief A disjunction of other Predicates.
 **/
class DisjunctionPredicate : public PredicateWithList {
 public:
  /**
   * @brief Constructor.
   **/
  DisjunctionPredicate()
      : PredicateWithList(), fresh_(true) {
  }

  serialization::Predicate getProto() const override;

  Predicate* clone() const override;

  PredicateType getPredicateType() const override {
    return kDisjunction;
  }

  bool matchesForSingleTuple(const ValueAccessor &accessor,
                             const tuple_id tuple) const override;

  bool matchesForJoinedTuples(
      const ValueAccessor &left_accessor,
      const relation_id left_relation_id,
      const tuple_id left_tuple_id,
      const ValueAccessor &right_accessor,
      const relation_id right_relation_id,
      const tuple_id right_tuple_id) const override;

  TupleIdSequence* getAllMatches(ValueAccessor *accessor,
                                 const SubBlocksReference *sub_blocks_ref,
                                 const TupleIdSequence *filter,
                                 const TupleIdSequence *existence_map) const override;

  /**
   * @brief Add a predicate to this disjunction.
   * @note If operand is another DusjunctionPredicate, it will be flattened
   *       into this Predicate, rather than nested beneath it.
   *
   * @param operand The predicate to add, becomes owned by this Predicate.
   **/
  void addPredicate(Predicate *operand);

 private:
  void processStaticOperand(const Predicate &operand);
  void processDynamicOperand();

  bool fresh_;

  DISALLOW_COPY_AND_ASSIGN(DisjunctionPredicate);
};

/** @} */

}  // namespace quickstep

#endif  // QUICKSTEP_EXPRESSIONS_PREDICATE_DISJUNCTION_PREDICATE_HPP_
