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

#ifndef QUICKSTEP_QUERY_EXECUTION_WORKER_MESSAGE_HPP_
#define QUICKSTEP_QUERY_EXECUTION_WORKER_MESSAGE_HPP_

#include <cstddef>

namespace quickstep {

class WorkOrder;
/**
 * @brief The messages to be sent to the worker from Foreman.
 *
 * @note  This class is copyable.
 **/
class WorkerMessage {
 public:
  enum WorkerMessageType {
    kRebuildWorkOrder = 0,
    kWorkOrder,
    kPoison
  };

  /**
   * @brief A static named constructor for generating rebuild WorkOrder messages.
   *
   * @param rebuild_workorder The rebuild WorkOrder to be executed by the worker.
   * @param relational_op_index The index of the relational operator in the
   *        query plan DAG that generated the given rebuild WorkOrder.
   **/
  static WorkerMessage RebuildWorkOrderMessage(WorkOrder *rebuild_workorder, std::size_t relational_op_index) {
    return WorkerMessage(rebuild_workorder, relational_op_index, kRebuildWorkOrder);
  }

  /**
   * @brief A static named constructor for generating WorkOrder messages.
   *
   * @param workorder The work order to be executed by the worker.
   * @param relational_op_index The index of the relational operator in the
   *                            query plan DAG that generated the given
   *                            workorder.
   **/
  static WorkerMessage WorkOrderMessage(WorkOrder *workorder, std::size_t relational_op_index) {
    return WorkerMessage(workorder, relational_op_index, kWorkOrder);
  }

  /**
   * @brief A static named constructor for generating a poison message.
   **/
  static WorkerMessage PoisonMessage() {
    return WorkerMessage(nullptr, 0, kPoison);
  }

  /**
   * @brief Destructor.
   **/
  ~WorkerMessage() {
  }

  /**
   * @brief Gets the work order to be executed by the worker.
   * @return A pointer to the work order which the worker should execute.
   **/
  inline WorkOrder* getWorkOrder() const {
    return work_unit_;
  }

  /**
   * @brief Get the index of the relational operator in the DAG that generated
   *        the workorder.
   **/
  inline const std::size_t getRelationalOpIndex() const {
    return relational_op_index_;
  }

  /**
   * @brief Get the type of this WorkerMessage.
   **/
  inline WorkerMessageType getType() const {
    return type_;
  }

 private:
  /**
   * @brief Constructor.
   *
   * @param work_unit The work order to be executed by the worker. A NULL
   *        workorder indicates a poison message.
   * @param relational_op_index The index of the relational operator in the
   *        query plan DAG that generated the given WorkOrder.
   * @param type Type of the WorkerMessage.
   **/
  WorkerMessage(WorkOrder *work_unit,
                const std::size_t relational_op_index,
                const WorkerMessageType type)
      : work_unit_(work_unit),
        relational_op_index_(relational_op_index),
        type_(type) {
  }

  WorkOrder *work_unit_;
  std::size_t relational_op_index_;
  WorkerMessageType type_;
};

}  // namespace quickstep

#endif  // QUICKSTEP_QUERY_EXECUTION_WORKER_MESSAGE_HPP_
