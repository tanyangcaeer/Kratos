//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Carlos A. Roig
//
//

#include "mpi.h"

#include "containers/model.h"
#include "includes/data_communicator.h"
#include "includes/model_part.h"
#include "mpi/utilities/parallel_fill_communicator.h"
#include "mpi/utilities/debug_utilities.h"

#include "testing/testing.h"

namespace Kratos {

namespace Testing {

KRATOS_DISTRIBUTED_TEST_CASE_IN_SUITE(DebugToolsCheckSingleVariableValue, KratosMPICoreFastSuite)
{
    const DataCommunicator& r_comm = DataCommunicator::GetDefault();
    const int world_rank = r_comm.Rank();
    const int world_size = r_comm.Size();

    Model model;
    ModelPart& model_part = model.CreateModelPart("ConsistentModelPart");

    model_part.AddNodalSolutionStepVariable(PRESSURE);          // Any variables will do
    model_part.AddNodalSolutionStepVariable(TEMPERATURE);       // Any variables will do
    model_part.AddNodalSolutionStepVariable(PARTITION_INDEX);

    // Put some nodes in every partition
    for(int i = 0; i < world_size; i++) {
        auto node = model_part.CreateNewNode(i, 0.0, 0.0, 0.0);

        node->FastGetSolutionStepValue(PRESSURE) = i%world_size;
        node->FastGetSolutionStepValue(TEMPERATURE) = (i%world_size)*10;
        node->FastGetSolutionStepValue(PARTITION_INDEX) = i%world_size;
    }

    // Build the communicator
    ParallelFillCommunicator(model_part).Execute();

    MpiDebugUtilities::CheckNodalHistoricalVariableConsistency(model_part, PRESSURE);
    MpiDebugUtilities::CheckNodalHistoricalVariableConsistency(model_part, TEMPERATURE);
}

// This will work with #5091 or when we move to C++17
// KRATOS_DISTRIBUTED_TEST_CASE_IN_SUITE(DebugToolsCheckMultipleVariablesValue, KratosMPICoreFastSuite)
// {
//     const DataCommunicator& r_comm = DataCommunicator::GetDefault();
//     const int world_rank = r_comm.Rank();
//     const int world_size = r_comm.Size();

//     Model model;
//     ModelPart& model_part = model.CreateModelPart("ConsistentModelPart");

//     model_part.AddNodalSolutionStepVariable(PRESSURE);          // Any variables will do
//     model_part.AddNodalSolutionStepVariable(TEMPERATURE);       // Any variables will do
//     model_part.AddNodalSolutionStepVariable(PARTITION_INDEX);

//     // Put some nodes in every partition
//     for(int i = 0; i < world_size; i++) {
//         auto node = model_part.CreateNewNode(i, 0.0, 0.0, 0.0);

//         node->FastGetSolutionStepValue(PRESSURE) = i%world_size;
//         node->FastGetSolutionStepValue(TEMPERATURE) = (i%world_size)*10;
//         node->FastGetSolutionStepValue(PARTITION_INDEX) = i%world_size;
//     }

//     // Build the communicator
//     ParallelFillCommunicator(model_part).Execute();

//     MpiDebugUtilities::CheckNodalHistoricalDatabaseConsistency(model_part);
// }

KRATOS_DISTRIBUTED_TEST_CASE_IN_SUITE(DebugToolsCheckSingleVariableFixity, KratosMPICoreFastSuite)
{
    const DataCommunicator& r_comm = DataCommunicator::GetDefault();
    const int world_rank = r_comm.Rank();
    const int world_size = r_comm.Size();

    Model model;
    ModelPart& model_part = model.CreateModelPart("ConsistentModelPart");

    model_part.AddNodalSolutionStepVariable(PRESSURE);          // Any variables will do
    model_part.AddNodalSolutionStepVariable(TEMPERATURE);       // Any variables will do
    model_part.AddNodalSolutionStepVariable(PARTITION_INDEX);

    // Put some nodes in every partition
    for(int i = 0; i < world_size; i++) {
        auto node = model_part.CreateNewNode(i, 0.0, 0.0, 0.0);

        node->FastGetSolutionStepValue(PRESSURE) = 0;
        node->FastGetSolutionStepValue(PARTITION_INDEX) = i%world_size;
    }

    // Build the communicator
    ParallelFillCommunicator(model_part).Execute();

    MpiDebugUtilities::CheckNodalHistoricalVariableConsistency(model_part, PRESSURE);
}

KRATOS_DISTRIBUTED_TEST_CASE_IN_SUITE(DebugToolsCheckSingleVariableValueError, KratosMPICoreFastSuite)
{
    const DataCommunicator& r_comm = DataCommunicator::GetDefault();
    const int world_rank = r_comm.Rank();
    const int world_size = r_comm.Size();

    Model model;
    ModelPart& model_part = model.CreateModelPart("ConsistentModelPart");

    model_part.AddNodalSolutionStepVariable(PRESSURE);          // Any variables will do
    model_part.AddNodalSolutionStepVariable(PARTITION_INDEX);

    // Put some nodes in every partition
    for(int i = 0; i < world_size; i++) {
        auto node = model_part.CreateNewNode(i, 0.0, 0.0, 0.0);

        node->FastGetSolutionStepValue(PRESSURE) = (world_rank == 0);
        node->FastGetSolutionStepValue(PARTITION_INDEX) = i%world_size;
    }

    // Build the communicator
    ParallelFillCommunicator(model_part).Execute();

    if(world_rank) {
        KRATOS_CHECK_EXCEPTION_IS_THROWN(
            MpiDebugUtilities::CheckNodalHistoricalVariableConsistency(model_part, PRESSURE),
            world_rank + " Inconsistent variable Val for Id: 0 Expected: 1 Obtained 0"
        );
    } else {
        KRATOS_CHECK_EXCEPTION_IS_THROWN(
            MpiDebugUtilities::CheckNodalHistoricalVariableConsistency(model_part, PRESSURE),
            world_rank + " Inconsistent variable Val for Id: 0 Expected: 0 Obtained 1"
        );
    }
}

}
}