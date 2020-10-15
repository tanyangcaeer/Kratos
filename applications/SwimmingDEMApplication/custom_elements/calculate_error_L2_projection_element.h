//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Joaquin Gonzalez-Usua
//


#if !defined(KRATOS_CALCULATE_ERROR_L2_PROJECTION_ELEMENT_H_INCLUDED )
#define  KRATOS_CALCULATE_ERROR_L2_PROJECTION_ELEMENT_H_INCLUDED

// System includes
#include <string>
#include <iostream>

// External includes

// Project includes
#include "containers/array_1d.h"
#include "includes/define.h"
#include "includes/element.h"
#include "includes/serializer.h"
#include "geometries/geometry.h"
#include "utilities/math_utils.h"
#include "utilities/geometry_utilities.h"
#include "includes/variables.h"

// Application includes
#include "fluid_dynamics_application_variables.h"
#include "swimming_dem_application_variables.h"


namespace Kratos
{

///@addtogroup SwimmingDEMApplication
///@{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/// A post-processing element to recover the Laplacian from the velocity solution.
/**
 */
template< unsigned int Dim, unsigned int NumNodes = Dim + 1 >
class KRATOS_API(SWIMMING_DEM_APPLICATION) CalculateErrorL2Projection : public Element
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of CalculateErrorL2Projection
    KRATOS_CLASS_POINTER_DEFINITION(CalculateErrorL2Projection);

    /// Node type (default is: Node<3>)
    typedef Node <3> NodeType;

    /// Geometry type (using with given NodeType)
    typedef Geometry<NodeType> GeometryType;

    /// Definition of nodes container type, redefined from GeometryType
    typedef Geometry<NodeType>::PointsArrayType NodesArrayType;

    /// Vector type for local contributions to the linear system
    typedef Vector VectorType;

    /// Matrix type for local contributions to the linear system
    typedef Matrix MatrixType;

    typedef Properties PropertiesType;

    typedef std::size_t IndexType;

    typedef std::size_t SizeType;

    typedef std::vector<std::size_t> EquationIdVectorType;

    typedef std::vector< Dof<double>::Pointer > DofsVectorType;

    typedef PointerVectorSet<Dof<double>, IndexedObject> DofsArrayType;

    /// Type for shape function values container
    typedef Kratos::Vector ShapeFunctionsType;

    ///@}
    ///@name Life Cycle
    ///@{

    //Constructors.

    /// Default constuctor.
    /**
     * @param NewId Index number of the new element (optional)
     */
    CalculateErrorL2Projection(IndexType NewId = 0) :
        Element(NewId), mCurrentComponent('X')
    {}

    /// Constructor using an array of nodes.
    /**
     * @param NewId Index of the new element
     * @param ThisNodes An array containing the nodes of the new element
     */
    CalculateErrorL2Projection(IndexType NewId, const NodesArrayType& ThisNodes) :
        Element(NewId, ThisNodes), mCurrentComponent('X')
    {}

    /// Constructor using a geometry object.
    /**
     * @param NewId Index of the new element
     * @param pGeometry Pointer to a geometry object
     */
    CalculateErrorL2Projection(IndexType NewId, GeometryType::Pointer pGeometry) :
        Element(NewId, pGeometry), mCurrentComponent('X')
    {}

    /// Constuctor using geometry and properties.
    /**
     * @param NewId Index of the new element
     * @param pGeometry Pointer to a geometry object
     * @param pProperties Pointer to the element's properties
     */
    CalculateErrorL2Projection(IndexType NewId, GeometryType::Pointer pGeometry, PropertiesType::Pointer pProperties) :
       Element(NewId, pGeometry, pProperties), mCurrentComponent('X')
    {}

    /// Destructor.
    virtual ~CalculateErrorL2Projection()
    {}


    ///@}
    ///@name Operators
    ///@{


    ///@}
    ///@name Operations
    ///@{

    /// Create a new element of this type
    /**
     * Returns a pointer to a new CalculateErrorL2Projection element, created using given input
     * @param NewId: the ID of the new element
     * @param ThisNodes: the nodes of the new element
     * @param pProperties: the properties assigned to the new element
     * @return a Pointer to the new element
     */
    Element::Pointer Create(
        IndexType NewId,
        NodesArrayType const& ThisNodes,
        PropertiesType::Pointer pProperties) const override
    {
        return Element::Pointer(new CalculateErrorL2Projection(NewId, this->GetGeometry().Create(ThisNodes), pProperties));
    }

    Element::Pointer Create(
        IndexType NewId,
        GeometryType::Pointer pGeom,
        PropertiesType::Pointer pProperties) const override
    {
        return Kratos::make_intrusive< CalculateErrorL2Projection >(NewId, pGeom, pProperties);
    }

    /// Calculate the element's local contribution to the system for the current step.
    virtual void CalculateLocalSystem(
        MatrixType& rLeftHandSideMatrix,
        VectorType& rRightHandSideVector,
        ProcessInfo& rCurrentProcessInfo) override;


    /// Provides the global indices for each one of this element's local rows
    /**
     * this determines the elemental equation ID vector for all elemental
     * DOFs
     * @param rResult A vector containing the global Id of each row
     * @param rCurrentProcessInfo the current process info object (unused)
     */
    virtual void EquationIdVector(
        EquationIdVectorType& rResult,
        ProcessInfo& rCurrentProcessInfo) override;

    /// Returns a list of the element's Dofs
    /**
     * @param ElementalDofList the list of DOFs
     * @param rCurrentProcessInfo the current process info instance
     */
    virtual void GetDofList(
        DofsVectorType& rElementalDofList,
        ProcessInfo& rCurrentProcessInfo) override;

    void CalculateMassMatrix(
        MatrixType& rMassMatrix,
        ProcessInfo& rCurrentProcessInfo) override;

    void AddConsistentMassMatrixContribution(
        MatrixType& rLHSMatrix,
        const array_1d<double,NumNodes>& rShapeFunc,
        const double Weight);

    void AddIntegrationPointRHSContribution(
        VectorType& rRHSVector,
        const array_1d<double,NumNodes>& rShapeFunc,
        const double Weight);

    void CalculateRHS(
        VectorType& rRHSVector,
        ProcessInfo& rCurrentProcessInfo);
    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Elemental Data
    ///@{

    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    virtual std::string Info() const override
    {
        std::stringstream buffer;
        buffer << "CalculateErrorL2Projection #" << this->Id();
        return buffer.str();
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << "CalculateErrorL2Projection" << Dim << "D";
    }

//        /// Print object's data.
//        virtual void PrintData(std::ostream& rOStream) const;

    ///@}
    ///@name Friends
    ///@{


    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{


    ///@}
    ///@name Protected member Variables
    ///@{
    char mCurrentComponent;

    ///@}
    ///@name Protected Operators
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
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    ///@}
    ///@name Serialization
    ///@{

    friend class Serializer;

    ///@}
    ///@name Private Operators
    ///@{

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
    CalculateErrorL2Projection & operator=(CalculateErrorL2Projection const& rOther);

    /// Copy constructor.
    CalculateErrorL2Projection(CalculateErrorL2Projection const& rOther);

    ///@}

}; // Class CalculateErrorL2Projection

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{


/// input stream function
template< unsigned int Dim >
inline std::istream& operator >>(std::istream& rIStream,
                                 CalculateErrorL2Projection<Dim>& rThis)
{
    return rIStream;
}

/// output stream function
template< unsigned int Dim >
inline std::ostream& operator <<(std::ostream& rOStream,
                                 const CalculateErrorL2Projection<Dim>& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}
///@}

///@} // Swimming DEM Application group

} // namespace Kratos.

#endif // KRATOS_CALCULATE_ERROR_L2_PROJECTION_ELEMENT_H_INCLUDED  defined