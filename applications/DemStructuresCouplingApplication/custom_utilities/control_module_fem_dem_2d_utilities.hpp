//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Ignasi de Pouplana
//


#ifndef KRATOS_CONTROL_MODULE_FEM_DEM_2D_UTILITIES
#define KRATOS_CONTROL_MODULE_FEM_DEM_2D_UTILITIES

// /* External includes */

// System includes

// Project includes
#include "includes/variables.h"

/* System includes */
#include <limits>
#include <iostream>
#include <iomanip>

/* External includes */
#ifdef _OPENMP
#include <omp.h>
#endif

/* Project includes */
#include "geometries/geometry.h"
#include "includes/define.h"
#include "includes/model_part.h"

#include "includes/table.h"
#include "includes/kratos_parameters.h"

// Application includes
#include "custom_elements/spheric_continuum_particle.h"

#include "dem_structures_coupling_application_variables.h"


namespace Kratos
{
class ControlModuleFemDem2DUtilities
{
public:

KRATOS_CLASS_POINTER_DEFINITION(ControlModuleFemDem2DUtilities);

/// Defining a table with double argument and result type as table type.
typedef Table<double,double> TableType;

/// Default constructor.

ControlModuleFemDem2DUtilities(ModelPart& rFemModelPart,
                            ModelPart& rDemModelPart,
                            Parameters& rParameters
                            ) :
                            mrFemModelPart(rFemModelPart),
                            mrDemModelPart(rDemModelPart)
{
    KRATOS_TRY

    Parameters default_parameters( R"(
        {
            "alternate_axis_loading": false,
            "target_stress_table_id" : 0,
            "initial_velocity" : 0.0,
            "limit_velocity" : 1.0,
            "velocity_factor" : 1.0,
            "compression_length" : 1.0,
            "face_area": 1.0,
            "young_modulus" : 1.0e7,
            "stress_increment_tolerance": 100.0,
            "update_stiffness": true,
            "start_time" : 0.0,
            "stress_averaging_time": 1.0e-5
        }  )" );

    // Now validate agains defaults -- this also ensures no type mismatch
    rParameters.ValidateAndAssignDefaults(default_parameters);

    // Note: this utility is design to be used in the Z direction of a 2D case
    mTargetStressTableId = rParameters["target_stress_table_id"].GetInt();
    mVelocity = rParameters["initial_velocity"].GetDouble();
    mLimitVelocity = rParameters["limit_velocity"].GetDouble();
    mVelocityFactor = rParameters["velocity_factor"].GetDouble();
    mCompressionLength = rParameters["compression_length"].GetDouble();
    mStartTime = rParameters["start_time"].GetDouble();
    mStressIncrementTolerance = rParameters["stress_increment_tolerance"].GetDouble();
    mUpdateStiffness = rParameters["update_stiffness"].GetBool();
    mReactionStressOld = 0.0;
    mStiffness = rParameters["young_modulus"].GetDouble()*rParameters["face_area"].GetDouble()/mCompressionLength;
    mStressAveragingTime = rParameters["stress_averaging_time"].GetDouble();
    mVectorOfLastStresses.resize(0);

    mAlternateAxisLoading = rParameters["alternate_axis_loading"].GetBool();
    mZCounter = 3;

    mrDemModelPart.GetProcessInfo()[TARGET_STRESS_Z] = 0.0;

    KRATOS_CATCH("");
}

/// Destructor.

virtual ~ControlModuleFemDem2DUtilities(){}

//***************************************************************************************************************
//***************************************************************************************************************

// Before FEM and DEM solution
void ExecuteInitialize()
{
    KRATOS_TRY;

    mrDemModelPart.GetProcessInfo()[IMPOSED_Z_STRAIN_VALUE] = 0.0;

    // Fem elements have IMPOSED_Z_STRAIN_VALUE already initialized as 0.0

    KRATOS_CATCH("");
}

// Before FEM and DEM solution
void ExecuteInitializeSolutionStep()
{
    KRATOS_TRY;

    // DEM variables
    ModelPart::ElementsContainerType& rElements = mrDemModelPart.GetCommunicator().LocalMesh().Elements();
    // FEM variables
    const double CurrentTime = mrFemModelPart.GetProcessInfo()[TIME];
    const double delta_time = mrFemModelPart.GetProcessInfo()[DELTA_TIME];
    const ProcessInfo& CurrentProcessInfo = mrFemModelPart.GetProcessInfo();
    int NElems = static_cast<int>(mrFemModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator elem_begin = mrFemModelPart.ElementsBegin();
    const int NNodes = static_cast<int>(mrFemModelPart.Nodes().size());
    ModelPart::NodesContainerType::iterator it_begin = mrFemModelPart.NodesBegin();
    TableType::Pointer pTargetStressTable = mrFemModelPart.pGetTable(mTargetStressTableId);

    // Calculate face_area
    double face_area = 0.0;
    // DEM modelpart
    #pragma omp parallel for reduction(+:face_area)
    for (int i = 0; i < (int)rElements.size(); i++) {
        ModelPart::ElementsContainerType::ptr_iterator ptr_itElem = rElements.ptr_begin() + i;
        Element* p_element = ptr_itElem->get();
        SphericContinuumParticle* pDemElem = dynamic_cast<SphericContinuumParticle*>(p_element);
        const double radius = pDemElem->GetRadius();
        face_area += Globals::Pi*radius*radius;
    }
    // FEM modelpart
    #pragma omp parallel for reduction(+:face_area)
    for(int i = 0; i < NElems; i++) {
        ModelPart::ElementsContainerType::iterator itElem = elem_begin + i;
        face_area += itElem->GetGeometry().Area();
    }

    // Calculate ReactionStress
    double face_reaction = 0.0;
    // DEM modelpart
    #pragma omp parallel for reduction(+:face_reaction)
    for (int i = 0; i < (int)rElements.size(); i++) {
        ModelPart::ElementsContainerType::ptr_iterator ptr_itElem = rElements.ptr_begin() + i;
        Element* p_element = ptr_itElem->get();
        SphericContinuumParticle* pDemElem = dynamic_cast<SphericContinuumParticle*>(p_element);
        BoundedMatrix<double, 3, 3> stress_tensor = ZeroMatrix(3,3);
        noalias(stress_tensor) = (*(pDemElem->mSymmStressTensor));
        const double radius = pDemElem->GetRadius();
        face_reaction += stress_tensor(2,2) * Globals::Pi*radius*radius;
    }
    // FEM modelpart
    #pragma omp parallel for reduction(+:face_reaction)
    for(int i = 0; i < NElems; i++)
    {
        ModelPart::ElementsContainerType::iterator itElem = elem_begin + i;
        Element::GeometryType& rGeom = itElem->GetGeometry();
        GeometryData::IntegrationMethod MyIntegrationMethod = itElem->GetIntegrationMethod();
        const Element::GeometryType::IntegrationPointsArrayType& IntegrationPoints = rGeom.IntegrationPoints(MyIntegrationMethod);
        unsigned int NumGPoints = IntegrationPoints.size();
        std::vector<Vector> stress_vector(NumGPoints);
        itElem->CalculateOnIntegrationPoints( CAUCHY_STRESS_VECTOR, stress_vector, CurrentProcessInfo );
        const double area_over_gp = rGeom.Area()/NumGPoints;
        // Loop through GaussPoints
        for ( unsigned int GPoint = 0; GPoint < NumGPoints; GPoint++ )
        {
            face_reaction += stress_vector[GPoint][2] * area_over_gp;
        }
    }
    double reaction_stress = face_reaction / face_area;

    reaction_stress = UpdateVectorOfHistoricalStressesAndComputeNewAverage(reaction_stress);

    // Update K if required
    double K_estimated = mStiffness;
    if(mUpdateStiffness == true) {
        if(std::abs(mVelocity) > 1.0e-4*std::abs(mLimitVelocity) &&
            std::abs(reaction_stress-mReactionStressOld) > mStressIncrementTolerance) {
            K_estimated = std::abs((reaction_stress-mReactionStressOld)/(mVelocity * delta_time));
        }
        mReactionStressOld = reaction_stress;
        mStiffness = K_estimated;
    }

    // Update velocity
    const double NextTargetStress = pTargetStressTable->GetValue(CurrentTime+delta_time);
    const double df_target = NextTargetStress - reaction_stress;
    double delta_velocity = df_target/(K_estimated * delta_time) - mVelocity;

    if(std::abs(df_target) < mStressIncrementTolerance) { delta_velocity = -mVelocity; }

    mVelocity += mVelocityFactor * delta_velocity;

    if(std::abs(mVelocity) > std::abs(mLimitVelocity)) {
        if(mVelocity >= 0.0) { mVelocity = std::abs(mLimitVelocity); }
        else { mVelocity = - std::abs(mLimitVelocity); }
    }

    const bool is_time_to_apply_cm = IsTimeToApplyCM();

    if (is_time_to_apply_cm == true)
    {
        // Update IMPOSED_Z_STRAIN_VALUE
        // DEM modelpart
        mrDemModelPart.GetProcessInfo()[IMPOSED_Z_STRAIN_VALUE] += mVelocity*delta_time/mCompressionLength;
        // FEM modelpart
        const double imposed_z_strain = mrDemModelPart.GetProcessInfo()[IMPOSED_Z_STRAIN_VALUE];
        #pragma omp parallel for
        for(int i = 0; i < NElems; i++)
        {
            ModelPart::ElementsContainerType::iterator itElem = elem_begin + i;
            Element::GeometryType& rGeom = itElem->GetGeometry();
            GeometryData::IntegrationMethod MyIntegrationMethod = itElem->GetIntegrationMethod();
            const Element::GeometryType::IntegrationPointsArrayType& IntegrationPoints = rGeom.IntegrationPoints(MyIntegrationMethod);
            unsigned int NumGPoints = IntegrationPoints.size();
            std::vector<double> imposed_z_strain_vector(NumGPoints);
            // Loop through GaussPoints
            for ( unsigned int GPoint = 0; GPoint < NumGPoints; GPoint++ )
            {
                imposed_z_strain_vector[GPoint] = imposed_z_strain;
            }
            itElem->SetValuesOnIntegrationPoints( IMPOSED_Z_STRAIN_VALUE, imposed_z_strain_vector, CurrentProcessInfo );
        }
    }

    // Save calculated velocity and reaction for print (only at FEM nodes)
    #pragma omp parallel for
    for(int i = 0; i<NNodes; i++) {
        ModelPart::NodesContainerType::iterator it = it_begin + i;
        it->FastGetSolutionStepValue(TARGET_STRESS_Z) = pTargetStressTable->GetValue(CurrentTime);
        it->FastGetSolutionStepValue(REACTION_STRESS_Z) = reaction_stress;
        it->FastGetSolutionStepValue(LOADING_VELOCITY_Z) = mVelocity;
    }

    mrDemModelPart.GetProcessInfo()[TARGET_STRESS_Z] = pTargetStressTable->GetValue(CurrentTime);

    KRATOS_CATCH("");
}


//***************************************************************************************************************
//***************************************************************************************************************

///@}
///@name Inquiry
///@{


///@}
///@name Input and output
///@{

/// Turn back information as a stemplate<class T, std::size_t dim> tring.

virtual std::string Info() const
{
    return "";
}

/// Print information about this object.

virtual void PrintInfo(std::ostream& rOStream) const
{
}

/// Print object's data.

virtual void PrintData(std::ostream& rOStream) const
{
}


///@}
///@name Friends
///@{

///@}

protected:
///@name Protected static Member r_variables
///@{

    ModelPart& mrFemModelPart;
    ModelPart& mrDemModelPart;
    unsigned int mTargetStressTableId;
    double mVelocity;
    double mLimitVelocity;
    double mVelocityFactor;
    double mCompressionLength;
    double mStartTime;
    double mReactionStressOld;
    double mStressIncrementTolerance;
    double mStiffness;
    bool mUpdateStiffness;
    std::vector<double> mVectorOfLastStresses;
    double mStressAveragingTime;
    bool mAlternateAxisLoading;
    unsigned int mZCounter;

///@}
///@name Protected member r_variables
///@{ template<class T, std::size_t dim>


///@}
///@name Protected Operators
///@{


///@}
///@name Protected Operations
///@{


///@}
///@name Protected  Access
///@{

///@}
///@name Protected Inquiry
///@{


///@}
///@name Protected LifeCycle
///@{


///@}

private:

///@name Static Member r_variables
///@{


///@}
///@name Member r_variables
///@{
///@}
///@name Private Operators
///@{

///@}
///@name Private Operations
///@{

double UpdateVectorOfHistoricalStressesAndComputeNewAverage(const double& last_reaction) {
    KRATOS_TRY;
    int length_of_vector = mVectorOfLastStresses.size();
    if (length_of_vector == 0) { //only the first time
        int number_of_steps_for_stress_averaging = (int) (mStressAveragingTime / mrFemModelPart.GetProcessInfo()[DELTA_TIME]);
        if(number_of_steps_for_stress_averaging < 1) number_of_steps_for_stress_averaging = 1;
        mVectorOfLastStresses.resize(number_of_steps_for_stress_averaging);
        KRATOS_INFO("DEM") << " 'number_of_steps_for_stress_averaging' is "<< number_of_steps_for_stress_averaging << std::endl;
    }

    length_of_vector = mVectorOfLastStresses.size();

    if(length_of_vector > 1) {
        for(int i=1; i<length_of_vector; i++) {
            mVectorOfLastStresses[i-1] = mVectorOfLastStresses[i];
        }
    }
    mVectorOfLastStresses[length_of_vector-1] = last_reaction;

    double average = 0.0;
    for(int i=0; i<length_of_vector; i++) {
        average += mVectorOfLastStresses[i];
    }
    average /= (double) length_of_vector;
    return average;

    KRATOS_CATCH("");
}

    bool IsTimeToApplyCM(){
        const double current_time = mrFemModelPart.GetProcessInfo()[TIME];
        bool apply_cm = false;

        if(current_time >= mStartTime) {
            if (mAlternateAxisLoading == true) {
                const unsigned int step = mrFemModelPart.GetProcessInfo()[STEP];
                if(step == mZCounter){
                    apply_cm = true;
                    mZCounter += 3;
                }
            } else {
                apply_cm = true;
            }
        }

        return apply_cm;
    }


///@}
///@name Private  Access
///@{


///@}
///@name Private Inquiry
///@{


///@}
///@name Un accessible methods
///@{

/// Assignment operator.
ControlModuleFemDem2DUtilities & operator=(ControlModuleFemDem2DUtilities const& rOther);


///@}

}; // Class ControlModuleFemDem2DUtilities

}  // namespace Python.

#endif // KRATOS_CONTROL_MODULE_FEM_DEM_2D_UTILITIES
