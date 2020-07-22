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

#if !defined(KRATOS_RANS_WALL_FUNCTION_UPDATE_PROCESS_H_INCLUDED)
#define KRATOS_RANS_WALL_FUNCTION_UPDATE_PROCESS_H_INCLUDED

// System includes
#include <string>

// External includes

// Project includes
#include "containers/model.h"
#include "processes/process.h"

namespace Kratos
{
///@addtogroup RANSApplication
///@{

///@name Kratos Classes
///@{

class KRATOS_API(RANS_APPLICATION) RansWallFunctionUpdateProcess : public Process
{
public:
    ///@name Type Definitions
    ///@{

    using NodeType = ModelPart::NodeType;
    using ConditionType = ModelPart::ConditionType;
    using ConditionGeometryType = ModelPart::ConditionType::GeometryType;

    /// Pointer definition of RansWallFunctionUpdateProcess
    KRATOS_CLASS_POINTER_DEFINITION(RansWallFunctionUpdateProcess);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor

    RansWallFunctionUpdateProcess(
        Model& rModel,
        Parameters rParameters);

    RansWallFunctionUpdateProcess(
        Model& rModel,
        const std::string& rModelPartName,
        const double VonKarman,
        const double Beta,
        const int EchoLevel);

    /// Destructor.
    ~RansWallFunctionUpdateProcess() override = default;

    ///@}
    ///@name Operations
    ///@{

    int Check() override;

    void ExecuteInitialize() override;

    void ExecuteInitializeSolutionStep() override;

    void Execute() override;

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override;

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override;

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override;

    ///@}

private:
    ///@name Member Variables
    ///@{

    Model& mrModel;
    std::string mModelPartName;
    double mVonKarman;
    double mBeta;
    double mCmu;
    int mEchoLevel;
    bool mIsInitialized = false;

    ///@}
    ///@name Private Operations
    ///@{

    void CalculateConditionNeighbourCount();

    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    RansWallFunctionUpdateProcess& operator=(RansWallFunctionUpdateProcess const& rOther);

    /// Copy constructor.
    RansWallFunctionUpdateProcess(RansWallFunctionUpdateProcess const& rOther);

    ///@}

}; // Class RansWallFunctionUpdateProcess

///@}
///@name Input and output
///@{

/// output stream function
inline std::ostream& operator<<(std::ostream& rOStream,
                                const RansWallFunctionUpdateProcess& rThis);

///@}

///@} addtogroup block

} // namespace Kratos.

#endif // KRATOS_RANS_WALL_FUNCTION_UPDATE_PROCESS_H_INCLUDED defined
