//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Inigo Lopez
//

// Project includes
#include "define_3d_wake_process.h"
#include "utilities/variable_utils.h"
#include "custom_utilities/potential_flow_utilities.h"
#include "processes/calculate_distance_to_skin_process.h"

namespace Kratos {

// Constructor for Define3DWakeProcess Process
Define3DWakeProcess::Define3DWakeProcess(ModelPart& rTrailingEdgeModelPart,
                                         ModelPart& rBodyModelPart,
                                         ModelPart& rStlWakeModelPart,
                                         const double Tolerance,
                                         const Vector& rWakeNormal)
    : Process(),
      mrTrailingEdgeModelPart(rTrailingEdgeModelPart),
      mrBodyModelPart(rBodyModelPart),
      mrStlWakeModelPart(rStlWakeModelPart),
      mTolerance(Tolerance),
      mWakeNormal(rWakeNormal)
{
    KRATOS_ERROR_IF(mWakeNormal.size() != 3)
        << "The mWakeNormal should be a vector with 3 components!"
        << std::endl;
}

void Define3DWakeProcess::ExecuteInitialize()
{
    InitializeTrailingEdgeSubModelpart();

    InitializeWakeSubModelpart();

    SetWakeAndSpanDirections();

    MarkTrailingEdgeNodes();

    ComputeLowerSurfaceNormals();

    MarkWakeElements();

    MarkKuttaElements();

    AddWakeNodes();
}

// This function initializes the variables and removes all of the elements of
// the trailing edge submodelpart
void Define3DWakeProcess::InitializeTrailingEdgeSubModelpart() const
{
    ModelPart& root_model_part = mrBodyModelPart.GetRootModelPart();
    if(root_model_part.HasSubModelPart("trailing_edge_elements_model_part"))
    {
        // Clearing the variables and elements of the already existing
        // trailing_edge_sub_model_part
        ModelPart& trailing_edge_sub_model_part =
            root_model_part.GetSubModelPart("trailing_edge_elements_model_part");

        for (auto& r_element : trailing_edge_sub_model_part.Elements()){
            r_element.SetValue(TRAILING_EDGE, false);
            r_element.SetValue(KUTTA, false);
            r_element.Reset(STRUCTURE);
            r_element.Set(TO_ERASE, true);
        }
        trailing_edge_sub_model_part.RemoveElements(TO_ERASE);
    }
    else{
        // Creating the trailing_edge_sub_model_part
        root_model_part.CreateSubModelPart("trailing_edge_elements_model_part");
    }
}

// This function initializes the variables and removes all of the elements of
// the wake submodelpart
void Define3DWakeProcess::InitializeWakeSubModelpart() const
{
    ModelPart& root_model_part = mrBodyModelPart.GetRootModelPart();
    if(root_model_part.HasSubModelPart("wake_elements_model_part"))
    {
        // Clearing the variables and elements of the already existing
        // wake_sub_model_part
        ModelPart& wake_sub_model_part =
            root_model_part.GetSubModelPart("wake_elements_model_part");

        for (auto& r_element : wake_sub_model_part.Elements()){
            r_element.SetValue(WAKE, false);
            r_element.SetValue(WAKE_ELEMENTAL_DISTANCES, ZeroVector(4));
            r_element.Set(TO_ERASE, true);
        }
        wake_sub_model_part.RemoveElements(TO_ERASE);
    }
    else{
        // Creating the wake_sub_model_part
        root_model_part.CreateSubModelPart("wake_elements_model_part");
    }
}

void Define3DWakeProcess::SetWakeAndSpanDirections()
{
    const auto free_stream_velocity = mrBodyModelPart.GetProcessInfo().GetValue(FREE_STREAM_VELOCITY);
    KRATOS_ERROR_IF(free_stream_velocity.size() != 3)
        << "The free stream velocity should be a vector with 3 components!"
        << std::endl;

    // Computing the norm of the free_stream_velocity vector
    const double norm = std::sqrt(inner_prod(free_stream_velocity, free_stream_velocity));

    const double eps = std::numeric_limits<double>::epsilon();
    KRATOS_ERROR_IF(norm < eps)
        << "The norm of the free stream velocity should be different than 0."
        << std::endl;

    // The wake direction is the free stream direction
    mWakeDirection = free_stream_velocity / norm;
    MathUtils<double>::CrossProduct(mSpanDirection, mWakeNormal, mWakeDirection);
}

void Define3DWakeProcess::MarkTrailingEdgeNodes()
{
    KRATOS_ERROR_IF(mrTrailingEdgeModelPart.NumberOfNodes() == 0) << "There are no nodes in the mrTrailingEdgeModelPart!"<< std::endl;

    for (auto& r_node : mrTrailingEdgeModelPart.Nodes()) {
        r_node.SetValue(TRAILING_EDGE, true);
    }
}

void Define3DWakeProcess::ComputeLowerSurfaceNormals() const
{
    #pragma omp parallel for
    for (int i = 0; i < static_cast<int>(mrBodyModelPart.Conditions().size()); i++) {
        ModelPart::ConditionIterator it_cond = mrBodyModelPart.ConditionsBegin() + i;

        auto r_geometry = it_cond->GetGeometry();
        auto surface_normal = r_geometry.UnitNormal(0);
        const double projection = inner_prod(surface_normal, mWakeNormal);

        if(projection > 0.0){
            for (unsigned int i = 0; i < it_cond->GetGeometry().size(); i++) {
                r_geometry[i].SetLock();
                r_geometry[i].SetValue(NORMAL, surface_normal);
                r_geometry[i].SetValue(LOWER_SURFACE, true);
                r_geometry[i].UnSetLock();
            }
        }
        else{
            for (unsigned int i = 0; i < it_cond->GetGeometry().size(); i++) {
                r_geometry[i].SetLock();
                r_geometry[i].SetValue(UPPER_SURFACE, true);
                r_geometry[i].UnSetLock();
            }
        }
    }

}

// This function checks which elements are cut by the wake and marks them as
// wake elements
void Define3DWakeProcess::MarkWakeElements()
{
    KRATOS_TRY;
    KRATOS_INFO("MarkWakeElements") << "...Selecting wake elements..." << std::endl;
    ModelPart& root_model_part = mrBodyModelPart.GetRootModelPart();
    std::vector<std::size_t> wake_elements_ordered_ids;

    CalculateDistanceToSkinProcess<3> distance_calculator(root_model_part, mrStlWakeModelPart);
    distance_calculator.Execute();

    #pragma omp parallel for
    for (int i = 0; i < static_cast<int>(root_model_part.Elements().size()); i++) {
        ModelPart::ElementIterator it_elem = root_model_part.ElementsBegin() + i;

        // Check if the element is touching the trailing edge
        CheckIfTrailingEdgeElement(*it_elem);

        if(it_elem->Is(TO_SPLIT)){
            it_elem->SetValue(WAKE, true);
            #pragma omp critical
            {
                wake_elements_ordered_ids.push_back(it_elem->Id());
            }
            auto wake_elemental_distances = it_elem->GetValue(ELEMENTAL_DISTANCES);
            auto r_geometry = it_elem->GetGeometry();
            for(unsigned int j = 0; j < wake_elemental_distances.size(); j++){
                if(std::abs(wake_elemental_distances[j] < mTolerance)){
                    if(wake_elemental_distances[j] < 0.0){
                        wake_elemental_distances[j] = - mTolerance;
                    }
                    else{
                        wake_elemental_distances[j] = mTolerance;
                    }
                }
                r_geometry[j].SetLock();
                r_geometry[j].SetValue(WAKE_DISTANCE, wake_elemental_distances[j]);
                r_geometry[j].UnSetLock();
            }
            it_elem->SetValue(WAKE_ELEMENTAL_DISTANCES, wake_elemental_distances);
        }
    }
    // Add the trailing edge elements to the trailing_edge_sub_model_part
    AddTrailingEdgeAndWakeElements(wake_elements_ordered_ids);
    KRATOS_INFO("MarkWakeElements") << "...Selecting wake elements finished..." << std::endl;
    KRATOS_CATCH("");
}

// This function checks if the element is touching the trailing edge
void Define3DWakeProcess::CheckIfTrailingEdgeElement(Element& rElement)
{
    // Loop over element nodes
    for (unsigned int i = 0; i < rElement.GetGeometry().size(); i++) {
        // Elements touching the trailing edge are trailing edge elements
        const auto& r_node = rElement.GetGeometry()[i];
        if (r_node.GetValue(TRAILING_EDGE)) {
            rElement.SetValue(TRAILING_EDGE, true);
            #pragma omp critical
            {
                mTrailingEdgeElementsOrderedIds.push_back(rElement.Id());
            }
        }
    }
}

// This function adds the trailing edge elements in the
// trailing_edge_sub_model_part
void Define3DWakeProcess::AddTrailingEdgeAndWakeElements(std::vector<std::size_t>& rWakeElementsOrderedIds)
{
    ModelPart& root_model_part = mrBodyModelPart.GetRootModelPart();

    std::sort(rWakeElementsOrderedIds.begin(),
              rWakeElementsOrderedIds.end());
    root_model_part.GetSubModelPart("wake_elements_model_part").AddElements(rWakeElementsOrderedIds);

    std::sort(mTrailingEdgeElementsOrderedIds.begin(),
              mTrailingEdgeElementsOrderedIds.end());
    root_model_part.GetSubModelPart("trailing_edge_elements_model_part").AddElements(mTrailingEdgeElementsOrderedIds);
}

// This function selects the kutta elements. Kutta elements are touching the
// trailing edge from below
void Define3DWakeProcess::MarkKuttaElements()
{
    KRATOS_INFO("MarkKuttaElements") << "...Selecting kutta elements..." << std::endl;
    ModelPart& root_model_part = mrBodyModelPart.GetRootModelPart();
    ModelPart& trailing_edge_sub_model_part =
        root_model_part.GetSubModelPart("trailing_edge_elements_model_part");

    std::vector<std::size_t> wake_elements_ordered_ids;

    #pragma omp parallel for
    for (int i = 0; i < static_cast<int>(trailing_edge_sub_model_part.Elements().size()); i++) {
        ModelPart::ElementsContainerType::ptr_iterator it_p_elem = trailing_edge_sub_model_part.Elements().ptr_begin() + i;
        auto p_elem = *it_p_elem;

        auto& r_geometry = p_elem->GetGeometry();
        // Selecting a trailing edge node
        NodeType::Pointer p_trailing_edge_node;
        unsigned int number_of_te_nodes = 0;
        for (unsigned int j = 0; j < r_geometry.size(); j++){
            const auto& r_node = r_geometry[j];
            if (r_node.GetValue(TRAILING_EDGE)){
                p_trailing_edge_node = r_geometry(j);
                number_of_te_nodes += 1;
            }
        }

        KRATOS_ERROR_IF(number_of_te_nodes < 0.5) << "Number of trailing edge nodes must be larger than 0 " << p_elem->Id()
        << " number_of_te_nodes = " << number_of_te_nodes << std::endl;

        const unsigned int number_of_non_te_nodes = 4 - number_of_te_nodes;

        Vector nodal_distances_to_te = ZeroVector(number_of_non_te_nodes);
        ComputeNodalDistancesToWakeAndLowerSurface(r_geometry, p_trailing_edge_node, nodal_distances_to_te);

        unsigned int number_of_nodes_with_negative_distance = 0;
        unsigned int number_of_nodes_with_positive_distance = 0;

        for(unsigned int j = 0; j < nodal_distances_to_te.size(); j++){
            if(nodal_distances_to_te[j] < 0.0){
                number_of_nodes_with_negative_distance += 1;
            }
            else{
                number_of_nodes_with_positive_distance +=1;
            }
        }

        if(number_of_nodes_with_negative_distance > number_of_non_te_nodes - 1){
            p_elem->SetValue(KUTTA, true);
            p_elem->SetValue(WAKE, false);
            p_elem->Set(TO_ERASE, true);
        }
        else if(number_of_nodes_with_positive_distance > 0 && number_of_nodes_with_negative_distance > 0){
            p_elem->Set(STRUCTURE);
            p_elem->SetValue(WAKE, true);
            BoundedVector<double, 4> wake_elemental_distances = ZeroVector(4);
            unsigned int counter = 0;
            for(unsigned int j = 0; j < r_geometry.size(); j++){
                const auto& r_node = r_geometry[j];
                if(r_node.GetValue(TRAILING_EDGE)){
                    wake_elemental_distances[j] = mTolerance;
                    r_geometry[j].SetLock();
                    r_geometry[j].SetValue(WAKE_DISTANCE, mTolerance);
                    r_geometry[j].UnSetLock();
                }
                else{
                    r_geometry[j].SetLock();
                    r_geometry[j].SetValue(WAKE_DISTANCE, nodal_distances_to_te[counter]);
                    r_geometry[j].UnSetLock();
                    wake_elemental_distances[j] = nodal_distances_to_te[counter];
                    counter += 1;
                }
            }
            p_elem->SetValue(WAKE_ELEMENTAL_DISTANCES, wake_elemental_distances);
        }
        else if(number_of_nodes_with_positive_distance > number_of_non_te_nodes - 1){
            p_elem->SetValue(WAKE, false);
            p_elem->Set(TO_ERASE, true);
        }
    }

    ModelPart& wake_sub_model_part = root_model_part.GetSubModelPart("wake_elements_model_part");
    wake_sub_model_part.RemoveElements(TO_ERASE);
    KRATOS_INFO("MarkKuttaElements") << "...Selecting kutta elements finished..." << std::endl;
}

void Define3DWakeProcess::ComputeNodalDistancesToWakeAndLowerSurface(const Element::GeometryType& rGeom, NodeType::Pointer pTrailingEdgeNode, Vector& rNodalDistancesToTe) const
{
    unsigned int counter = 0;
    for (unsigned int i = 0; i < rGeom.size(); i++){
        const auto& r_node = rGeom[i];
        if (!r_node.GetValue(TRAILING_EDGE)){
            // Compute the distance vector from the trailing edge to the node
            const array_1d<double,3> distance_vector = r_node.Coordinates() - pTrailingEdgeNode->Coordinates();

            // Compute the distance in the free stream direction
            const double free_stream_direction_distance = inner_prod(distance_vector, mWakeDirection);

            double distance;
            if(r_node.GetValue(UPPER_SURFACE)){
                distance = mTolerance;
            }
            else if(r_node.GetValue(LOWER_SURFACE)){
                distance = - mTolerance;
            }
            else if(free_stream_direction_distance < 0.0){
                distance = inner_prod(distance_vector, pTrailingEdgeNode->GetValue(NORMAL));
            }
            else{
                distance = inner_prod(distance_vector, mWakeNormal);
            }

            if(std::abs(distance) < mTolerance){
                distance = - mTolerance;
            }

            rNodalDistancesToTe[counter] = distance;
            counter += 1;
        }
    }
}

void Define3DWakeProcess::AddWakeNodes() const
{
    ModelPart& root_model_part = mrBodyModelPart.GetRootModelPart();
    ModelPart& wake_sub_model_part =
            root_model_part.GetSubModelPart("wake_elements_model_part");

    std::vector<std::size_t> wake_nodes_ordered_ids;
    for (auto& r_element : wake_sub_model_part.Elements()){
        for (unsigned int i = 0; i < r_element.GetGeometry().size(); i++){
            r_element.GetGeometry()[i].SetValue(WAKE, true);
            wake_nodes_ordered_ids.push_back(r_element.GetGeometry()[i].Id());
        }
    }

    std::sort(wake_nodes_ordered_ids.begin(),
              wake_nodes_ordered_ids.end());
    wake_sub_model_part.AddNodes(wake_nodes_ordered_ids);

}
} // namespace Kratos.
