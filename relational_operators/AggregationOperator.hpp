/**
 * This file copyright (c) 2011-2015, Quickstep Technologies LLC.
 * Copyright (c) 2015, Pivotal Software, Inc.
 * All rights reserved.
 * See file CREDITS.txt for details.
 **/

#ifndef QUICKSTEP_RELATIONAL_OPERATORS_AGGREGATION_OPERATOR_HPP_
#define QUICKSTEP_RELATIONAL_OPERATORS_AGGREGATION_OPERATOR_HPP_

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
class StorageManager;
class WorkOrdersContainer;

/** \addtogroup RelationalOperators
 *  @{
 */

/**
 * @brief An operator which performs aggregation over a relation.
 **/
class AggregationOperator : public RelationalOperator {
 public:
  /**
   * @brief Constructor for aggregating with arbitrary expressions in projection
   *        list.
   *
   * @param input_relation The relation to perform aggregation over.
   * @param input_relation_is_stored If input_relation is a stored relation and
   *        is fully available to the operator before it can start generating
   *        workorders.
   * @param aggr_state_index The index of the AggregationState in QueryContext.
   **/
  AggregationOperator(const CatalogRelation &input_relation,
                      bool input_relation_is_stored,
                      const QueryContext::aggregation_state_id aggr_state_index)
      : input_relation_is_stored_(input_relation_is_stored),
        input_relation_block_ids_(input_relation_is_stored ? input_relation.getBlocksSnapshot()
                                                           : std::vector<block_id>()),
        aggr_state_index_(aggr_state_index),
        num_workorders_generated_(0),
        started_(false) {}

  ~AggregationOperator() override {}

  bool getAllWorkOrders(WorkOrdersContainer *container) override;

  void feedInputBlock(const block_id input_block_id, const relation_id input_relation_id) override {
    input_relation_block_ids_.push_back(input_block_id);
  }

  void feedInputBlocks(const relation_id rel_id, std::vector<block_id> *partially_filled_blocks) override {
    input_relation_block_ids_.insert(input_relation_block_ids_.end(),
                                     partially_filled_blocks->begin(),
                                     partially_filled_blocks->end());
  }

 private:
  const bool input_relation_is_stored_;
  std::vector<block_id> input_relation_block_ids_;
  const QueryContext::aggregation_state_id aggr_state_index_;

  std::vector<block_id>::size_type num_workorders_generated_;
  bool started_;

  DISALLOW_COPY_AND_ASSIGN(AggregationOperator);
};

/**
 * @brief A WorkOrder produced by AggregationOperator.
 **/
class AggregationWorkOrder : public WorkOrder {
 public:
  /**
   * @brief Constructor
   *
   * @param input_block_id The block id.
   * @param aggr_state_index The index of the AggregationState in QueryContext.
   **/
  AggregationWorkOrder(
      const block_id input_block_id,
      const QueryContext::aggregation_state_id aggr_state_index)
      : input_block_id_(input_block_id),
        aggr_state_index_(aggr_state_index) {}

  ~AggregationWorkOrder() override {}

  void execute(QueryContext *query_context,
               CatalogDatabase *catalog_database,
               StorageManager *storage_manager) override;

 private:
  const block_id input_block_id_;
  const QueryContext::aggregation_state_id aggr_state_index_;

  DISALLOW_COPY_AND_ASSIGN(AggregationWorkOrder);
};

/** @} */

}  // namespace quickstep

#endif  // QUICKSTEP_RELATIONAL_OPERATORS_AGGREGATION_OPERATOR_HPP_
