/**
 * This file copyright (c) 2011-2015, Quickstep Technologies LLC.
 * Copyright (c) 2015, Pivotal Software, Inc.
 * All rights reserved.
 * See file CREDITS.txt for details.
 **/

#include "query_optimizer/QueryProcessor.hpp"

#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>

#include "catalog/Catalog.hpp"
#include "catalog/Catalog.pb.h"
#include "catalog/CatalogDatabase.hpp"
#include "catalog/CatalogRelation.hpp"
#include "parser/ParseStatement.hpp"
#include "query_optimizer/Optimizer.hpp"
#include "query_optimizer/QueryHandle.hpp"
#include "storage/StorageManager.hpp"

using std::ifstream;
using std::ofstream;

namespace quickstep {

QueryHandle* QueryProcessor::generateQueryHandle(const ParseStatement &statement) {
  std::unique_ptr<QueryHandle> query_handle(new QueryHandle());

  optimizer::Optimizer optimizer(foreman_client_id_, query_id_, getDefaultDatabase(), storage_manager_.get(), bus_);
  optimizer.generateQueryHandle(statement, query_handle.get());

  if (optimizer.isCatalogChanged() && !catalog_altered_) {
    catalog_altered_ = true;
  }

  ++query_id_;

  return query_handle.release();
}

void QueryProcessor::saveCatalog() {
  if (catalog_altered_) {
    ofstream catalog_file(catalog_filename_.c_str());

    if (!catalog_->getProto().SerializeToOstream(&catalog_file)) {
      throw UnableToWriteCatalog(catalog_filename_);
    }

    catalog_file.close();

    catalog_altered_ = false;
  }
}

void QueryProcessor::loadCatalog() {
  ifstream catalog_file(catalog_filename_.c_str());
  if (!catalog_file.good()) {
    throw UnableToReadCatalog(catalog_filename_);
  }

  serialization::Catalog catalog_proto;
  if (!catalog_proto.ParseFromIstream(&catalog_file)) {
    throw CatalogNotProto(catalog_filename_);
  }
  catalog_file.close();
  catalog_.reset(new Catalog(catalog_proto));

  catalog_altered_ = false;
}

}  // namespace quickstep
