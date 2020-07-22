//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya
//

// System includes
#include <cmath>

/* Project includes */
#include "includes/define.h"
#include "spaces/ublas_space.h"
#include "utilities/openmp_utils.h"

// Include base h
#include "generic_convergence_criteria.h"

namespace Kratos
{
template <>
void GenericConvergenceCriteria<UblasSpace<double, CompressedMatrix, Vector>, UblasSpace<double, Matrix, Vector>>::CalculateConvergenceCheckNorms(
    double& rSolutionNorm,
    double& rIncreaseNorm,
    double& rDofSize,
    ModelPart& rModelPart,
    DofsArrayType& rDofSet,
    const TSystemMatrixType& rA,
    const TSystemVectorType& rDx,
    const TSystemVectorType& rb)
{
    KRATOS_TRY

    int number_of_dofs = rDofSet.size();

    double solution_norm{0.0}, increase_norm{0.0};
    int dof_num{0};

    // Set a partition for OpenMP
    PartitionVector dof_partition;
    const int number_of_threads = OpenMPUtils::GetNumThreads();
    OpenMPUtils::DivideInPartitions(number_of_dofs, number_of_threads, dof_partition);

    // Loop over Dofs
#pragma omp parallel reduction(+ : solution_norm, increase_norm, dof_num)
    {
        const int k = OpenMPUtils::ThisThread();
        typename DofsArrayType::iterator dof_begin = rDofSet.begin() + dof_partition[k];
        typename DofsArrayType::iterator dof_end =
            rDofSet.begin() + dof_partition[k + 1];

        std::size_t dof_id;
        TDataType dof_value;
        TDataType dof_increment;

        for (typename DofsArrayType::iterator itDof = dof_begin; itDof != dof_end; ++itDof) {
            if (itDof->IsFree()) {
                dof_id = itDof->EquationId();
                dof_value = itDof->GetSolutionStepValue(0);
                dof_increment = rDx[dof_id];

                solution_norm += dof_value * dof_value;
                increase_norm += dof_increment * dof_increment;
                dof_num += 1;
            }
        }
    }

    rSolutionNorm = std::sqrt(solution_norm);
    rIncreaseNorm = std::sqrt(increase_norm);
    rDofSize = static_cast<double>(dof_num);

    KRATOS_CATCH("");
}

// template instantiations

template class GenericConvergenceCriteria<UblasSpace<double, CompressedMatrix, Vector>,
                                          UblasSpace<double, Matrix, Vector>>;

} // namespace Kratos
