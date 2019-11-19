//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Inigo Lopez

#include "custom_utilities/potential_flow_utilities.h"
#include "compressible_potential_flow_application_variables.h"
#include "includes/model_part.h"
#include <fstream>

namespace Kratos {
namespace PotentialFlowUtilities {
template <int Dim, int NumNodes>
array_1d<double, NumNodes> GetWakeDistances(const Element& rElement)
{
    return rElement.GetValue(WAKE_ELEMENTAL_DISTANCES);
}

template <int Dim, int NumNodes>
BoundedVector<double, NumNodes> GetPotentialOnNormalElement(const Element& rElement)
{
    const int kutta = rElement.GetValue(KUTTA);
    array_1d<double, NumNodes> potentials;

    const auto r_geometry = rElement.GetGeometry();

    if (kutta == 0) {
        for (unsigned int i = 0; i < NumNodes; i++) {
            potentials[i] = r_geometry[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL);
        }
    }
    else {
        for (unsigned int i = 0; i < NumNodes; i++) {
            if (!r_geometry[i].GetValue(TRAILING_EDGE)) {
                potentials[i] = r_geometry[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL);
            }
            else {
                potentials[i] = r_geometry[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL);
                //potentials[i] = r_geometry[i].FastGetSolutionStepValue(PSI);
            }
        }
    }

    return potentials;
}

template <int Dim, int NumNodes>
BoundedVector<double, 2 * NumNodes> GetPotentialOnWakeElement(
    const Element& rElement, const array_1d<double, NumNodes>& rDistances)
{
    const auto upper_potentials =
        GetPotentialOnUpperWakeElement<Dim, NumNodes>(rElement, rDistances);

    const auto lower_potentials =
        GetPotentialOnLowerWakeElement<Dim, NumNodes>(rElement, rDistances);

    BoundedVector<double, 2 * NumNodes> split_element_values;
    for (unsigned int i = 0; i < NumNodes; i++) {
        split_element_values[i] = upper_potentials[i];
        split_element_values[NumNodes + i] = lower_potentials[i];
    }

    return split_element_values;
}

template <int Dim, int NumNodes>
BoundedVector<double, NumNodes> GetPotentialOnUpperWakeElement(
    const Element& rElement, const array_1d<double, NumNodes>& rDistances)
{
    array_1d<double, NumNodes> upper_potentials;
    const auto r_geometry = rElement.GetGeometry();

    for (unsigned int i = 0; i < NumNodes; i++){
        if (rDistances[i] > 0.0){
            upper_potentials[i] = r_geometry[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL);
        }
        else{
            upper_potentials[i] = r_geometry[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL);
        }
    }

    return upper_potentials;
}

template <int Dim, int NumNodes>
BoundedVector<double, NumNodes> GetPotentialOnLowerWakeElement(
    const Element& rElement, const array_1d<double, NumNodes>& rDistances)
{
    array_1d<double, NumNodes> lower_potentials;
    const auto r_geometry = rElement.GetGeometry();

    for (unsigned int i = 0; i < NumNodes; i++){
        if (rDistances[i] < 0.0){
            lower_potentials[i] = r_geometry[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL);
        }
        else{
            lower_potentials[i] = r_geometry[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL);
            // if(r_geometry[i].GetValue(TRAILING_EDGE)){
            //     lower_potentials[i] = r_geometry[i].FastGetSolutionStepValue(PSI);
            // }
            // else{
            //     lower_potentials[i] = r_geometry[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL);
            // }
        }
    }

    return lower_potentials;
}

template <int Dim, int NumNodes>
array_1d<double, Dim> ComputeVelocity(const Element& rElement)
{
    const int wake = rElement.GetValue(WAKE);

    if (wake == 0)
        return ComputeVelocityNormalElement<Dim,NumNodes>(rElement);
    else
        return ComputeVelocityUpperWakeElement<Dim,NumNodes>(rElement);
}

template <int Dim, int NumNodes>
array_1d<double, Dim> ComputeVelocityNormalElement(const Element& rElement)
{
    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(rElement.GetGeometry(), data.DN_DX, data.N, data.vol);

    data.potentials = GetPotentialOnNormalElement<Dim,NumNodes>(rElement);

    return prod(trans(data.DN_DX), data.potentials);
}

template <int Dim, int NumNodes>
array_1d<double, Dim> ComputeVelocityUpperWakeElement(const Element& rElement)
{
    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(rElement.GetGeometry(), data.DN_DX, data.N, data.vol);

    const auto& r_distances = GetWakeDistances<Dim,NumNodes>(rElement);

    data.potentials = GetPotentialOnUpperWakeElement<Dim,NumNodes>(rElement, r_distances);

    return prod(trans(data.DN_DX), data.potentials);
}

template <int Dim, int NumNodes>
array_1d<double, Dim> ComputeVelocityLowerWakeElement(const Element& rElement)
{
    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(rElement.GetGeometry(), data.DN_DX, data.N, data.vol);

    const auto& r_distances = GetWakeDistances<Dim,NumNodes>(rElement);

    data.potentials = GetPotentialOnLowerWakeElement<Dim,NumNodes>(rElement, r_distances);

    return prod(trans(data.DN_DX), data.potentials);
}

template <int Dim, int NumNodes>
double ComputeIncompressiblePressureCoefficient(const Element& rElement, const ProcessInfo& rCurrentProcessInfo)
{
    const array_1d<double, 3> free_stream_velocity = rCurrentProcessInfo[FREE_STREAM_VELOCITY];
    const double free_stream_velocity_norm = inner_prod(free_stream_velocity, free_stream_velocity);

    KRATOS_ERROR_IF(free_stream_velocity_norm < std::numeric_limits<double>::epsilon())
        << "Error on element -> " << rElement.Id() << "\n"
        << "free_stream_velocity_norm must be larger than zero." << std::endl;

    array_1d<double, Dim> v = ComputeVelocity<Dim,NumNodes>(rElement);

    double pressure_coefficient = (free_stream_velocity_norm - inner_prod(v, v)) /
               free_stream_velocity_norm; // 0.5*(norm_2(free_stream_velocity) - norm_2(v));
    return pressure_coefficient;
}


template <int Dim, int NumNodes>
double ComputeCompressiblePressureCoefficient(const Element& rElement, const ProcessInfo& rCurrentProcessInfo)
{
    // Reading free stream conditions
    const array_1d<double, 3>& vinfinity = rCurrentProcessInfo[FREE_STREAM_VELOCITY];
    const double M_inf = rCurrentProcessInfo[FREE_STREAM_MACH];
    const double heat_capacity_ratio = rCurrentProcessInfo[HEAT_CAPACITY_RATIO];

    // Computing local velocity
    array_1d<double, Dim> v = ComputeVelocity<Dim, NumNodes>(rElement);

    // Computing squares
    const double v_inf_2 = inner_prod(vinfinity, vinfinity);
    const double M_inf_2 = M_inf * M_inf;
    double v_2 = inner_prod(v, v);

    KRATOS_ERROR_IF(v_inf_2 < std::numeric_limits<double>::epsilon())
        << "Error on element -> " << rElement.Id() << "\n"
        << "v_inf_2 must be larger than zero." << std::endl;

    const double base = 1 + (heat_capacity_ratio - 1) * M_inf_2 * (1 - v_2 / v_inf_2) / 2;

    return 2 * (pow(base, heat_capacity_ratio / (heat_capacity_ratio - 1)) - 1) /
           (heat_capacity_ratio * M_inf_2);
}

template <int Dim, int NumNodes>
const bool CheckIfElementIsCutByDistance(const BoundedVector<double, NumNodes>& rNodalDistances)
{
    // Initialize counters
    unsigned int number_of_nodes_with_positive_distance = 0;
    unsigned int number_of_nodes_with_negative_distance = 0;

    // Count how many element nodes are above and below the wake
    for (unsigned int i = 0; i < rNodalDistances.size(); i++) {
        if (rNodalDistances(i) < 0.0) {
            number_of_nodes_with_negative_distance += 1;
        }
        else {
            number_of_nodes_with_positive_distance += 1;
        }
    }

    // Elements with nodes above and below the wake are wake elements
    return number_of_nodes_with_negative_distance > 0 &&
           number_of_nodes_with_positive_distance > 0;
}

template <int Dim>
void CheckIfWakeConditionsAreFulfilled(const ModelPart& rWakeModelPart, const double& rTolerance, const int& rEchoLevel)
{
    unsigned int number_of_unfulfilled_wake_conditions = 0;
    unsigned int number_of_unfulfilled_wing_tip_wake_conditions = 0;
    const double absolute_tolerance = rTolerance;
    const double relative_tolerance = rTolerance;
    BoundedVector<int, Dim> number_of_unfulfilled_wake_conditions_absolute_tolerance = ZeroVector(Dim);
    BoundedVector<int, Dim> number_of_unfulfilled_wake_conditions_relative_tolerance = ZeroVector(Dim);
    for (auto& r_element : rWakeModelPart.Elements()){
        const auto upper_velocity = ComputeVelocityUpperWakeElement<Dim,Dim+1>(r_element);
        const auto lower_velocity = ComputeVelocityLowerWakeElement<Dim,Dim+1>(r_element);

        for (unsigned int i = 0; i < Dim; i++){
            double reference = std::abs(upper_velocity[i]);
            if(reference < 1e-8){
                reference = 1e-8;
            }

            double absolute_error;
            if(i == 1){
                absolute_error = std::abs(upper_velocity[i] + lower_velocity[i]);
            }
            else{
                absolute_error = std::abs(upper_velocity[i] - lower_velocity[i]);

            }
            const double relative_error = absolute_error/reference;

            if(absolute_error > absolute_tolerance){
                number_of_unfulfilled_wake_conditions_absolute_tolerance[i] += 1;
            }
            if(relative_error > relative_tolerance){
                number_of_unfulfilled_wake_conditions_relative_tolerance[i] += 1;
                // KRATOS_WARNING_IF("CheckWakeCondition", rEchoLevel > 0)
                //     << "WAKE CONDITION NOT FULFILLED WITH A RELATIVE TOLERANCE OF " << relative_tolerance
                //     << " IN ELEMENT # " << r_element.Id() << std::endl;
            }
        }

        const bool wake_condition_is_fulfilled =
            CheckWakeCondition<Dim, Dim + 1>(r_element, rTolerance, rEchoLevel);
        if (!wake_condition_is_fulfilled)
        {
            number_of_unfulfilled_wake_conditions += 1;
            if(r_element.GetValue(WING_TIP)){
                number_of_unfulfilled_wing_tip_wake_conditions +=1;
            }
        }
    }

    const double percentage_unfulfilled = number_of_unfulfilled_wake_conditions * 100.0 / rWakeModelPart.NumberOfElements();

    KRATOS_WARNING_IF("\nCheckIfWakeConditionsAreFulfilled", number_of_unfulfilled_wake_conditions > 0)
        << " THE WAKE CONDITION IS NOT FULFILLED IN " << percentage_unfulfilled
        << " % OF THE WAKE ELEMENTS WITH AN ABSOLUTE TOLERANCE OF " << rTolerance << std::endl;

    KRATOS_WARNING_IF("\nCheckIfWakeConditionsAreFulfilled", number_of_unfulfilled_wake_conditions > 0)
        << " THE WAKE CONDITION IS NOT FULFILLED IN " << number_of_unfulfilled_wake_conditions
        << " OF " << rWakeModelPart.NumberOfElements()
        << " OUT OF WHICH WING TIP ARE " << number_of_unfulfilled_wing_tip_wake_conditions
        << " ELEMENTS WITH AN ABSOLUTE TOLERANCE OF " << rTolerance << std::endl;

    for (unsigned int i = 0; i < Dim; i++){
        KRATOS_WARNING_IF("CheckIfWakeConditionsAreFulfilled", number_of_unfulfilled_wake_conditions_absolute_tolerance[i] > 0)
            << " THE WAKE CONDITION IS NOT FULFILLED IN THE " << i
            << " COMPONENT IN " << number_of_unfulfilled_wake_conditions_absolute_tolerance[i]
            << " ELEMENTS WITH AN ABSOLUTE TOLERANCE OF " << absolute_tolerance << std::endl;
    }

    for (unsigned int i = 0; i < Dim; i++){
        KRATOS_WARNING_IF("CheckIfWakeConditionsAreFulfilled", number_of_unfulfilled_wake_conditions_relative_tolerance[i] > 0)
            << " THE WAKE CONDITION IS NOT FULFILLED IN THE " << i
            << " COMPONENT IN " << number_of_unfulfilled_wake_conditions_relative_tolerance[i]
            << " OF " << rWakeModelPart.NumberOfElements()
            << " ELEMENTS WITH A RELATIVE TOLERANCE OF " << relative_tolerance << std::endl;
    }
}

template <int Dim, int NumNodes>
const bool CheckWakeCondition(const Element& rElement, const double& rTolerance, const int& rEchoLevel)
{
    const auto upper_velocity = ComputeVelocityUpperWakeElement<Dim,NumNodes>(rElement);
    const auto lower_velocity = ComputeVelocityLowerWakeElement<Dim,NumNodes>(rElement);

    bool wake_condition_is_fulfilled = true;
    if(std::abs(upper_velocity[0] - lower_velocity[0]) > rTolerance){
        wake_condition_is_fulfilled = false;
    }
    if(std::abs(upper_velocity[2] - lower_velocity[2]) > rTolerance){
        wake_condition_is_fulfilled = false;
    }
    // for (unsigned int i = 0; i < upper_velocity.size(); i++){
    //     if(std::abs(upper_velocity[i] - lower_velocity[i]) > rTolerance){
    //         wake_condition_is_fulfilled = false;
    //         break;
    //     }
    // }

    KRATOS_WARNING_IF("CheckWakeCondition", !wake_condition_is_fulfilled && rEchoLevel > 2)
        << "WAKE CONDITION NOT FULFILLED IN ELEMENT WING TIP" << rElement.GetValue(WING_TIP)
        << " WITH ID # " << rElement.Id() << std::endl;
    KRATOS_WARNING_IF("CheckWakeCondition", !wake_condition_is_fulfilled && rEchoLevel > 3)
        << "WAKE CONDITION NOT FULFILLED IN ELEMENT # " << rElement.Id()
        << " upper_velocity  = " << upper_velocity
        << " lower_velocity  = " << lower_velocity << std::endl;
    // if(!wake_condition_is_fulfilled && rEchoLevel > 1){
    //     std::ofstream outfile;
    //     outfile.open("unfulfilled_wake_elements_id.txt", std::ios_base::app);
    //     outfile << rElement.Id();
    //     outfile << "\n";
    // }

    return wake_condition_is_fulfilled;
}

template <int Dim>
void CheckIfPressureEqualityWakeConditionsAreFulfilled(const ModelPart& rWakeModelPart, const double& rTolerance, const int& rEchoLevel)
{
    std::ofstream outfile;
    outfile.open("unfulfilled_pressure_wake_elements_id.txt");
    unsigned int number_of_unfulfilled_wake_conditions = 0;
    unsigned int number_of_unfulfilled_wing_tip_wake_conditions = 0;
    unsigned int number_of_unfulfilled_zero_velocity_wake_conditions = 0;
    unsigned int number_of_unfulfilled_structure_wake_conditions = 0;
    for (auto& r_element : rWakeModelPart.Elements()){
        const bool wake_condition_is_fulfilled =
            CheckPressureEqualityWakeCondition<Dim, Dim + 1>(r_element, rTolerance, rEchoLevel);
        if (!wake_condition_is_fulfilled)
        {
            number_of_unfulfilled_wake_conditions += 1;
            if(r_element.GetValue(WING_TIP)){
                number_of_unfulfilled_wing_tip_wake_conditions += 1;
                number_of_unfulfilled_structure_wake_conditions += 1;
            }
            else if(r_element.GetValue(ZERO_VELOCITY_CONDITION)){
                number_of_unfulfilled_zero_velocity_wake_conditions += 1;
                number_of_unfulfilled_structure_wake_conditions += 1;
            }
        }
    }
    const double percentage_unfulfilled = number_of_unfulfilled_wake_conditions * 100.0 / rWakeModelPart.NumberOfElements();

    KRATOS_WARNING_IF("\nCheckIfPressureEqualityWakeConditionsAreFulfilled", number_of_unfulfilled_wake_conditions > 0)
        << " THE PRESSURE EQUALITY WAKE CONDITION IS NOT FULFILLED IN " << percentage_unfulfilled
        << " % OF THE WAKE ELEMENTS WITH AN ABSOLUTE TOLERANCE OF " << rTolerance << std::endl;


    KRATOS_WARNING_IF("CheckIfPressureEqualityWakeConditionsAreFulfilled", number_of_unfulfilled_wake_conditions > 0)
        << "THE PRESSURE EQUALITY WAKE CONDITION IS NOT FULFILLED IN " << number_of_unfulfilled_wake_conditions
        << " OF " << rWakeModelPart.NumberOfElements()
        << " ELEMENTS WITH AN ABSOLUTE TOLERANCE OF " << rTolerance << std::endl;

    KRATOS_WARNING_IF("CheckIfPressureEqualityWakeConditionsAreFulfilled", number_of_unfulfilled_wake_conditions > 0)
        << "THE PRESSURE EQUALITY WAKE CONDITION IS NOT FULFILLED IN " << number_of_unfulfilled_structure_wake_conditions
        << " STRUCTURE ELEMENTS WITH AN ABSOLUTE TOLERANCE OF " << rTolerance << std::endl;

    KRATOS_WARNING_IF("CheckIfPressureEqualityWakeConditionsAreFulfilled", number_of_unfulfilled_wake_conditions > 0)
        << "THE PRESSURE EQUALITY WAKE CONDITION IS NOT FULFILLED IN " << number_of_unfulfilled_wing_tip_wake_conditions
        << " WING_TIP ELEMENTS WITH AN ABSOLUTE TOLERANCE OF " << rTolerance << std::endl;

    KRATOS_WARNING_IF("CheckIfPressureEqualityWakeConditionsAreFulfilled", number_of_unfulfilled_wake_conditions > 0)
        << "THE PRESSURE EQUALITY WAKE CONDITION IS NOT FULFILLED IN " << number_of_unfulfilled_zero_velocity_wake_conditions
        << " ZERO VELOCITY ELEMENTS WITH AN ABSOLUTE TOLERANCE OF " << rTolerance << std::endl;
}

template <int Dim, int NumNodes>
const bool CheckPressureEqualityWakeCondition(const Element& rElement, const double& rTolerance, const int& rEchoLevel)
{
    const auto upper_velocity = ComputeVelocityUpperWakeElement<Dim,NumNodes>(rElement);
    const auto lower_velocity = ComputeVelocityLowerWakeElement<Dim,NumNodes>(rElement);

    const double upper_velocity_2 = inner_prod(upper_velocity, upper_velocity);
    const double lower_velocity_2 = inner_prod(lower_velocity, lower_velocity);

    bool wake_condition_is_fulfilled = true;
    if(std::abs(upper_velocity_2 - lower_velocity_2) > rTolerance){
            wake_condition_is_fulfilled = false;
    }

    KRATOS_WARNING_IF("CheckWakeCondition", !wake_condition_is_fulfilled && rEchoLevel > 2)
        << "WAKE CONDITION NOT FULFILLED IN ELEMENT # " << rElement.Id() << std::endl;
    KRATOS_WARNING_IF("CheckWakeCondition", !wake_condition_is_fulfilled && rEchoLevel > 3)
        << "WAKE CONDITION NOT FULFILLED IN ELEMENT # " << rElement.Id()
        << " upper_velocity  = " << upper_velocity
        << " lower_velocity  = " << lower_velocity << std::endl;

    if(!wake_condition_is_fulfilled && rEchoLevel > 1){
        std::ofstream outfile;
        outfile.open("unfulfilled_pressure_wake_elements_id.txt", std::ios_base::app);
        outfile << rElement.Id();
        outfile << "\n";
    }

    return wake_condition_is_fulfilled;
}

template <int Dim, int NumNodes>
const bool CheckWakeConditionXDirection(const Element& rElement, const int& rComponent, const double& rTolerance, const int& rEchoLevel)
{
    const auto upper_velocity = ComputeVelocityUpperWakeElement<Dim,NumNodes>(rElement);
    const auto lower_velocity = ComputeVelocityLowerWakeElement<Dim,NumNodes>(rElement);

    double reference = std::abs(upper_velocity[rComponent]);
    if(reference < 1e-8){
        reference = 1e-8;
    }

    const double absolute_error = std::abs(upper_velocity[rComponent] - lower_velocity[rComponent]);
    const double relative_error = absolute_error/reference;

    bool wake_condition_is_fulfilled = true;
    if(absolute_error > rTolerance){
        wake_condition_is_fulfilled = false;
    }

    KRATOS_WARNING_IF("CheckWakeCondition", !wake_condition_is_fulfilled && rEchoLevel > 0)
        << "WAKE CONDITION NOT FULFILLED WITH AN ABSOLUTE ERROR = " << absolute_error
        << "AND A RELATIVE ERROR = " << relative_error
        << " IN THE " << rComponent << "COMPONENT, IN ELEMENT WING TIP" << rElement.GetValue(WING_TIP)
        << " WITH ID # " << rElement.Id() << std::endl;
    KRATOS_WARNING_IF("CheckWakeCondition", !wake_condition_is_fulfilled && rEchoLevel > 1)
        << "WAKE CONDITION NOT FULFILLED IN ELEMENT # " << rElement.Id()
        << " upper_velocity  = " << upper_velocity
        << " lower_velocity  = " << lower_velocity << std::endl;

    return wake_condition_is_fulfilled;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Template instantiation

// 2D
template array_1d<double, 3> GetWakeDistances<2, 3>(const Element& rElement);
template BoundedVector<double, 3> GetPotentialOnNormalElement<2, 3>(const Element& rElement);
template BoundedVector<double, 2 * 3> GetPotentialOnWakeElement<2, 3>(
    const Element& rElement, const array_1d<double, 3>& rDistances);
template BoundedVector<double, 3> GetPotentialOnUpperWakeElement<2, 3>(
    const Element& rElement, const array_1d<double, 3>& rDistances);
template BoundedVector<double, 3> GetPotentialOnLowerWakeElement<2, 3>(
    const Element& rElement, const array_1d<double, 3>& rDistances);
template array_1d<double, 2> ComputeVelocityNormalElement<2, 3>(const Element& rElement);
template array_1d<double, 2> ComputeVelocityUpperWakeElement<2, 3>(const Element& rElement);
template array_1d<double, 2> ComputeVelocityLowerWakeElement<2, 3>(const Element& rElement);
template array_1d<double, 2> ComputeVelocity<2, 3>(const Element& rElement);
template double ComputeIncompressiblePressureCoefficient<2, 3>(const Element& rElement, const ProcessInfo& rCurrentProcessInfo);
template double ComputeCompressiblePressureCoefficient<2, 3>(const Element& rElement, const ProcessInfo& rCurrentProcessInfo);
template const bool CheckIfElementIsCutByDistance<2, 3>(const BoundedVector<double, 3>& rNodalDistances);
template void KRATOS_API(COMPRESSIBLE_POTENTIAL_FLOW_APPLICATION) CheckIfWakeConditionsAreFulfilled<2>(const ModelPart&, const double& rTolerance, const int& rEchoLevel);
template void KRATOS_API(COMPRESSIBLE_POTENTIAL_FLOW_APPLICATION) CheckIfPressureEqualityWakeConditionsAreFulfilled<2>(const ModelPart&, const double& rTolerance, const int& rEchoLevel);
template const bool CheckWakeCondition<2, 3>(const Element& rElement, const double& rTolerance, const int& rEchoLevel);
template const bool CheckPressureEqualityWakeCondition<2, 3>(const Element& rElement, const double& rTolerance, const int& rEchoLevel);
template const bool CheckWakeConditionXDirection<2, 3>(const Element& rElement, const int& rComponent, const double& rTolerance, const int& rEchoLevel);

// 3D
template array_1d<double, 4> GetWakeDistances<3, 4>(const Element& rElement);
template BoundedVector<double, 4> GetPotentialOnNormalElement<3, 4>(const Element& rElement);
template BoundedVector<double, 2 * 4> GetPotentialOnWakeElement<3, 4>(
    const Element& rElement, const array_1d<double, 4>& rDistances);
template BoundedVector<double, 4> GetPotentialOnUpperWakeElement<3, 4>(
    const Element& rElement, const array_1d<double, 4>& rDistances);
template BoundedVector<double, 4> GetPotentialOnLowerWakeElement<3, 4>(
    const Element& rElement, const array_1d<double, 4>& rDistances);
template array_1d<double, 3> ComputeVelocityNormalElement<3, 4>(const Element& rElement);
template array_1d<double, 3> ComputeVelocityUpperWakeElement<3, 4>(const Element& rElement);
template array_1d<double, 3> ComputeVelocityLowerWakeElement<3, 4>(const Element& rElement);
template array_1d<double, 3> ComputeVelocity<3, 4>(const Element& rElement);
template double ComputeIncompressiblePressureCoefficient<3, 4>(const Element& rElement, const ProcessInfo& rCurrentProcessInfo);
template double ComputeCompressiblePressureCoefficient<3, 4>(const Element& rElement, const ProcessInfo& rCurrentProcessInfo);
template const bool CheckIfElementIsCutByDistance<3, 4>(const BoundedVector<double, 4>& rNodalDistances);
template void  KRATOS_API(COMPRESSIBLE_POTENTIAL_FLOW_APPLICATION) CheckIfWakeConditionsAreFulfilled<3>(const ModelPart&, const double& rTolerance, const int& rEchoLevel);
template void  KRATOS_API(COMPRESSIBLE_POTENTIAL_FLOW_APPLICATION) CheckIfPressureEqualityWakeConditionsAreFulfilled<3>(const ModelPart&, const double& rTolerance, const int& rEchoLevel);
template const bool CheckWakeCondition<3, 4>(const Element& rElement, const double& rTolerance, const int& rEchoLevel);
template const bool CheckPressureEqualityWakeCondition<3, 4>(const Element& rElement, const double& rTolerance, const int& rEchoLevel);
template const bool CheckWakeConditionXDirection<3, 4>(const Element& rElement, const int& rComponent, const double& rTolerance, const int& rEchoLevel);
} // namespace PotentialFlow
} // namespace Kratos
