/**
 *   Copyright 2011-2015 Quickstep Technologies LLC.
 *   Copyright 2015 Pivotal Software, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 **/

#ifndef QUICKSTEP_RELATIONAL_OPERATORS_SELECT_OPERATOR_HPP_
#define QUICKSTEP_RELATIONAL_OPERATORS_SELECT_OPERATOR_HPP_

#include <cstddef>
#include <memory>
#include <vector>

#include "catalog/CatalogRelation.hpp"
#include "catalog/CatalogTypedefs.hpp"
#include "query_execution/QueryContext.hpp"
#include "relational_operators/RelationalOperator.hpp"
#include "relational_operators/WorkOrder.hpp"
#include "storage/StorageBlockInfo.hpp"
#include "utility/Macros.hpp"

namespace quickstep {

class CatalogDatabase;
class CatalogRelationSchema;
class StorageManager;
class WorkOrdersContainer;

/** \addtogroup RelationalOperators
 *  @{
 */

/**
 * @brief An operator which performs simple selection over a relation.
 **/
class SelectOperator : public RelationalOperator {
 public:
  /**
   * @brief Constructor for selection with arbitrary expressions in projection
   *        list.
   *
   * @param input_relation The relation to perform selection over.
   * @param output_relation_id The id of the output relation.
   * @param output_destination_index The index of the InsertDestination in the
   *        QueryContext to insert the selection results.
   * @param predicate_index The index of selection predicate in QueryContext.
   *        All tuples matching pred will be selected (or kInvalidPredicateId to
   *        select all tuples).
   * @param selection_index The group index of Scalars in QueryContext, which
   *        will be evaluated to project input tuples.
   * @param input_relation_is_stored If input_relation is a stored relation and
   *        is fully available to the operator before it can start generating
   *        workorders.
   **/
  SelectOperator(const CatalogRelation &input_relation,
                 const relation_id output_relation_id,
                 const QueryContext::insert_destination_id output_destination_index,
                 const QueryContext::predicate_id predicate_index,
                 const QueryContext::scalar_group_id selection_index,
                 bool input_relation_is_stored)
      : input_relation_(input_relation),
        output_relation_id_(output_relation_id),
        output_destination_index_(output_destination_index),
        predicate_index_(predicate_index),
        selection_index_(selection_index),
        simple_selection_(nullptr),
        input_relation_block_ids_(input_relation_is_stored ? input_relation.getBlocksSnapshot()
                                                           : std::vector<block_id>()),
        num_workorders_generated_(0),
        simple_projection_(false),
        input_relation_is_stored_(input_relation_is_stored),
        started_(false) {}

  /**
   * @brief Constructor for selection with simple projection of attributes.
   *
   * @note selection_index_ is invalid, and will not be used for projection.
   *
   * @param input_relation The relation to perform selection over.
   * @param output_relation_id The id of the output relation.
   * @param output_destination_index The index of the InsertDestination in the
   *        QueryContext to insert the selection results.
   * @param selection A projection list of attribute IDs. The operator takes
   *        ownership of selection.
   * @param predicate_index The index of selection predicate in QueryContext.
   *        All tuples matching pred will be selected (or kInvalidPredicateId to
   *        select all tuples).
   * @param input_relation_is_stored If input_relation is a stored relation and
   *        is fully available to the operator before it can start generating
   *        workorders.
   **/
  SelectOperator(const CatalogRelation &input_relation,
                 const relation_id output_relation_id,
                 const QueryContext::insert_destination_id output_destination_index,
                 const QueryContext::predicate_id predicate_index,
                 std::vector<attribute_id> *selection,
                 bool input_relation_is_stored)
      : input_relation_(input_relation),
        output_relation_id_(output_relation_id),
        output_destination_index_(output_destination_index),
        predicate_index_(predicate_index),
        selection_index_(QueryContext::kInvalidScalarGroupId),
        simple_selection_(selection),
        input_relation_block_ids_(input_relation_is_stored ? input_relation.getBlocksSnapshot()
                                                           : std::vector<block_id>()),
        num_workorders_generated_(0),
        simple_projection_(true),
        input_relation_is_stored_(input_relation_is_stored),
        started_(false) {}

  ~SelectOperator() override {}

  bool getAllWorkOrders(WorkOrdersContainer *container) override;

  void feedInputBlock(const block_id input_block_id, const relation_id input_relation_id) override {
    input_relation_block_ids_.push_back(input_block_id);
  }

  void feedInputBlocks(const relation_id rel_id, std::vector<block_id> *partially_filled_blocks) override {
    input_relation_block_ids_.insert(input_relation_block_ids_.end(),
                                     partially_filled_blocks->begin(),
                                     partially_filled_blocks->end());
  }

  QueryContext::insert_destination_id getInsertDestinationID() const override {
    return output_destination_index_;
  }

  const relation_id getOutputRelationID() const override {
    return output_relation_id_;
  }

 private:
  const CatalogRelationSchema &input_relation_;

  const relation_id output_relation_id_;
  const QueryContext::insert_destination_id output_destination_index_;
  const QueryContext::predicate_id predicate_index_;

  const QueryContext::scalar_group_id selection_index_;
  std::unique_ptr<std::vector<attribute_id> > simple_selection_;

  std::vector<block_id> input_relation_block_ids_;

  // A single workorder is generated for each block of input relation.
  std::vector<block_id>::size_type num_workorders_generated_;

  const bool simple_projection_;
  const bool input_relation_is_stored_;
  bool started_;

  DISALLOW_COPY_AND_ASSIGN(SelectOperator);
};

/**
 * @brief A WorkOrder produced by SelectOperator.
 **/
class SelectWorkOrder : public WorkOrder {
 public:
  /**
   * @brief Constructor.
   *
   * @note Reference parameters simple_selection and selection are NOT owned by
   *       this class and must remain valid until after execute() is called.
   *
   * @param input_relation_id The id of the relation to perform selection over.
   * @param output_destination_index The index of the InsertDestination in the
   *        QueryContext to insert the selection results.
   * @param predicate_index The index of selection predicate in QueryContext.
   *        All tuples matching pred will be selected (or kInvalidPredicateId to
   *        select all tuples).
   * @param selection_index The group index of Scalars in QueryContext, which
   *        will be evaluated to project input tuples.
   * @param simple_selection The list of attribute ids, used if
   *        simple_projection is true.
   * @param simple_projection Whether the Select is simple.
   * @param input_block_id The block id.
   **/
  SelectWorkOrder(const relation_id input_relation_id,
                  const QueryContext::insert_destination_id output_destination_index,
                  const QueryContext::predicate_id predicate_index,
                  const QueryContext::scalar_group_id selection_index,
                  const std::vector<attribute_id> &simple_selection,
                  const bool simple_projection,
                  const block_id input_block_id)
      : input_relation_id_(input_relation_id),
        output_destination_index_(output_destination_index),
        predicate_index_(predicate_index),
        selection_index_(selection_index),
        simple_selection_(simple_selection),
        simple_projection_(simple_projection),
        input_block_id_(input_block_id) {}

  ~SelectWorkOrder() override {}

  /**
   * @exception TupleTooLargeForBlock A tuple produced by this selection was
   *            too large to insert into an empty block provided by
   *            output_destination_index_ in query_context. Select may be
   *            partially complete (with some tuples inserted into the
   *            destination) when this exception is thrown, causing potential
   *            inconsistency.
   **/
  void execute(QueryContext *query_context,
               CatalogDatabase *catalog_database,
               StorageManager *storage_manager) override;

 private:
  const relation_id input_relation_id_;
  const QueryContext::insert_destination_id output_destination_index_;
  const QueryContext::predicate_id predicate_index_;

  const QueryContext::scalar_group_id selection_index_;
  const std::vector<attribute_id> &simple_selection_;
  const bool simple_projection_;

  const block_id input_block_id_;

  DISALLOW_COPY_AND_ASSIGN(SelectWorkOrder);
};

/** @} */

}  // namespace quickstep

#endif  // QUICKSTEP_RELATIONAL_OPERATORS_SELECT_OPERATOR_HPP_
