/**
 * This file copyright (c) 2011-2015, Quickstep Technologies LLC.
 * Copyright (c) 2015, Pivotal Software, Inc.
 * All rights reserved.
 * See file CREDITS.txt for details.
 **/

#include "relational_operators/SaveBlocksOperator.hpp"

#include <vector>

#include "query_execution/WorkOrdersContainer.hpp"
#include "storage/StorageBlockInfo.hpp"
#include "storage/StorageManager.hpp"

#include "glog/logging.h"

namespace quickstep {

bool SaveBlocksOperator::getAllWorkOrders(WorkOrdersContainer *container) {
  while (num_workorders_generated_ < destination_block_ids_.size()) {
    container->addNormalWorkOrder(
        new SaveBlocksWorkOrder(
            destination_block_ids_[num_workorders_generated_],
            force_),
        op_index_);
    ++num_workorders_generated_;
  }
  return done_feeding_input_relation_;
}

void SaveBlocksOperator::feedInputBlock(const block_id input_block_id, const relation_id input_relation_id) {
  destination_block_ids_.push_back(input_block_id);
}

void SaveBlocksWorkOrder::execute(QueryContext *query_context,
                                  CatalogDatabase *catalog_database,
                                  StorageManager *storage_manager) {
  DCHECK(storage_manager != nullptr);

  // It may happen that the block gets saved to disk as a result of an eviction,
  // before this invocation. In either case, we don't care about the return
  // value of saveBlockOrBlob.
  storage_manager->saveBlockOrBlob(save_block_id_, force_);
}

}  // namespace quickstep
