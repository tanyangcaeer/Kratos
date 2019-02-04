// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:        BSD License
//	                license: structural_mechanics_application/license.txt
//
//  Main authors:    Armin Geiser
//

// System includes

// External includes
#include <pybind11/stl.h>

// Project includes
#include "custom_python/add_custom_response_functions_to_python.h"

// Processes
#include "custom_response_functions/adjoint_processes/replace_elements_and_conditions_for_adjoint_problem_process.h"

// Response Functions
#include "custom_response_functions/response_utilities/strain_energy_response_function_utility.h"
#include "custom_response_functions/response_utilities/mass_response_function_utility.h"
#include "custom_response_functions/response_utilities/eigenfrequency_response_function_utility.h"

#include "response_functions/adjoint_response_function.h"
#include "custom_response_functions/response_utilities/adjoint_local_stress_response_function.h"
#include "custom_response_functions/response_utilities/adjoint_nodal_displacement_response_function.h"
#include "custom_response_functions/response_utilities/adjoint_linear_strain_energy_response_function.h"

#include "state_derivative/response_functions/direct_sensitivity_response_function.cpp"
#include "state_derivative/response_functions/direct_sensitivity_local_stress_response_function.cpp"
#include "state_derivative/response_functions/direct_sensitivity_nodal_displacement_response_function.cpp"

// Direct Sensitivity variables
#include "state_derivative/variable_utilities/direct_sensitivity_variable.h"
#include "state_derivative/variable_utilities/direct_sensitivity_element_data_variable.h"

// Adjoint postprocessing
#include "custom_response_functions/response_utilities/adjoint_postprocess.h"

// Direct sensitivity postprocess 
#include "state_derivative/variable_utilities/direct_sensitivity_postprocess.h"

namespace Kratos {
namespace Python {

void  AddCustomResponseFunctionUtilitiesToPython(pybind11::module& m)
{
    namespace py = pybind11;

    // Response Functions
    py::class_<StrainEnergyResponseFunctionUtility, StrainEnergyResponseFunctionUtility::Pointer >
        (m, "StrainEnergyResponseFunctionUtility")
        .def(py::init<ModelPart&, Parameters>())
        .def("Initialize", &StrainEnergyResponseFunctionUtility::Initialize)
        .def("CalculateValue", &StrainEnergyResponseFunctionUtility::CalculateValue)
        .def("CalculateGradient", &StrainEnergyResponseFunctionUtility::CalculateGradient);

    py::class_<MassResponseFunctionUtility, MassResponseFunctionUtility::Pointer >
        (m, "MassResponseFunctionUtility")
        .def(py::init<ModelPart&, Parameters>())
        .def("Initialize", &MassResponseFunctionUtility::Initialize)
        .def("CalculateValue", &MassResponseFunctionUtility::CalculateValue)
        .def("CalculateGradient", &MassResponseFunctionUtility::CalculateGradient);

    py::class_<EigenfrequencyResponseFunctionUtility, EigenfrequencyResponseFunctionUtility::Pointer >
        (m, "EigenfrequencyResponseFunctionUtility")
        .def(py::init<ModelPart&, Parameters>())
        .def("Initialize", &EigenfrequencyResponseFunctionUtility::Initialize)
        .def("CalculateValue", &EigenfrequencyResponseFunctionUtility::CalculateValue)
        .def("CalculateGradient", &EigenfrequencyResponseFunctionUtility::CalculateGradient);

    // Processes
    py::class_<ReplaceElementsAndConditionsForAdjointProblemProcess, ReplaceElementsAndConditionsForAdjointProblemProcess::Pointer , Process>
        (m, "ReplaceElementsAndConditionsForAdjointProblemProcess")
        .def(py::init<ModelPart&>());

    // Variable classes
    py::class_<DirectSensitivityVariable, DirectSensitivityVariable::Pointer>
        (m, "DirectSensitivityVariable")
        .def(py::init<ModelPart&, Parameters>())
        .def("Initialize", &DirectSensitivityVariable::Initialize)
        .def("InitializeSolutionStep", &DirectSensitivityVariable::InitializeSolutionStep)
        .def("FinalizeSolutionStep", &DirectSensitivityVariable::FinalizeSolutionStep);        

    py::class_<DirectSensitivityElementDataVariable, DirectSensitivityElementDataVariable::Pointer, DirectSensitivityVariable>
        (m, "DirectSensitivityElementDataVariable")
        .def(py::init<ModelPart&, Parameters>());
    
    // Response Functions
    py::class_<AdjointLocalStressResponseFunction, AdjointLocalStressResponseFunction::Pointer, AdjointResponseFunction>
        (m, "AdjointLocalStressResponseFunction")
        .def(py::init<ModelPart&, Parameters>());

    py::class_<AdjointNodalDisplacementResponseFunction, AdjointNodalDisplacementResponseFunction::Pointer, AdjointResponseFunction>
        (m, "AdjointNodalDisplacementResponseFunction")
        .def(py::init<ModelPart&, Parameters>());

    py::class_<AdjointLinearStrainEnergyResponseFunction, AdjointLinearStrainEnergyResponseFunction::Pointer, AdjointResponseFunction>
        (m, "AdjointLinearStrainEnergyResponseFunction")
        .def(py::init<ModelPart&, Parameters>());

    py::class_<DirectSensitivityResponseFunction, DirectSensitivityResponseFunction::Pointer>
        (m, "DirectSensitivityResponseFunction")
        .def(py::init<ModelPart&, Parameters>());

    py::class_<DirectSensitivityLocalStressResponseFunction, DirectSensitivityLocalStressResponseFunction::Pointer, DirectSensitivityResponseFunction >
        (m, "DirectSensitivityLocalStressResponseFunction")
        .def(py::init<ModelPart&, Parameters>());

    py::class_<DirectSensitivityNodalDisplacementResponseFunction, DirectSensitivityNodalDisplacementResponseFunction::Pointer, DirectSensitivityResponseFunction >
        (m, "DirectSensitivityNodalDisplacementResponseFunction")
        .def(py::init<ModelPart&, Parameters>());
     
    // Adjoint postprocess
    py::class_<AdjointPostprocess, AdjointPostprocess::Pointer>
      (m, "AdjointPostprocess")
      .def(py::init<ModelPart&, AdjointResponseFunction&, Parameters>())
      .def("Initialize", &AdjointPostprocess::Initialize)
      .def("InitializeSolutionStep", &AdjointPostprocess::InitializeSolutionStep)
      .def("FinalizeSolutionStep", &AdjointPostprocess::FinalizeSolutionStep)
      .def("UpdateSensitivities", &AdjointPostprocess::UpdateSensitivities);

    // Direct sensitivity postprocess
    py::class_<DirectSensitivityPostprocess, DirectSensitivityPostprocess::Pointer>
      (m, "DirectSensitivityPostprocess")
      .def(py::init<ModelPart&, DirectSensitivityResponseFunction&, DirectSensitivityVariable&, Parameters>())
      .def("Initialize", &DirectSensitivityPostprocess::Initialize)
      .def("InitializeSolutionStep", &DirectSensitivityPostprocess::InitializeSolutionStep)
      .def("FinalizeSolutionStep", &DirectSensitivityPostprocess::FinalizeSolutionStep)
      .def("UpdateSensitivities", &DirectSensitivityPostprocess::UpdateSensitivities);
}

}  // namespace Python.
} // Namespace Kratos

