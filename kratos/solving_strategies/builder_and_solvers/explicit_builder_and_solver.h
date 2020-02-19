//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                     Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//
//

#if !defined(KRATOS_EXPLICIT_BUILDER_AND_SOLVER )
#define  KRATOS_EXPLICIT_BUILDER_AND_SOLVER

/* System includes */
#include <set>
#include <unordered_set>

/* External includes */

/* Project includes */
#include "includes/define.h"
#include "includes/model_part.h"
#include "utilities/openmp_utils.h"

namespace Kratos
{
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

/**
 * @class ExplicitBuilderAndSolver
 * @ingroup KratosCore
 * @brief Current class provides an implementation for the base explicit builder and solving operations.
 * @details The RHS is constituted by the unbalanced loads (residual)
 * Degrees of freedom are reordered putting the restrained degrees of freedom at
 * the end of the system ordered in reverse order with respect to the DofSet.
 * @author Ruben Zorrilla
 */
template<class TSparseSpace, class TDenseSpace >
class ExplicitBuilderAndSolver
{
public:
    ///@name Type Definitions
    ///@{

    /// Definition of the size type
    typedef std::size_t SizeType;

    /// Definition of the index type
    typedef std::size_t IndexType;

    /// Definition of the data type
    typedef typename TSparseSpace::DataType TDataType;

    ///Definition of the sparse matrix
    typedef typename TSparseSpace::MatrixType TSystemMatrixType;

    /// Definition of the vector size
    typedef typename TSparseSpace::VectorType TSystemVectorType;

    /// Definition of the pointer to the sparse matrix
    typedef typename TSparseSpace::MatrixPointerType TSystemMatrixPointerType;

    /// Definition of the pointer to the vector
    typedef typename TSparseSpace::VectorPointerType TSystemVectorPointerType;

    /// The local matrix definition
    typedef typename TDenseSpace::MatrixType LocalSystemMatrixType;

    /// The local vector definition
    typedef typename TDenseSpace::VectorType LocalSystemVectorType;

    // /// Definition of the scheme type
    // typedef Scheme<TSparseSpace, TDenseSpace> TSchemeType;

    /// Definition of the DoF class
    typedef ModelPart::DofType TDofType;

    /// Definition of the DoF array type
    typedef ModelPart::DofsArrayType DofsArrayType;

    /// Definition of the DoF vector type
    typedef ModelPart::DofsVectorType DofsVectorType;

    /// The definition of the DoF objects
    typedef typename DofsArrayType::iterator DofIteratorType;
    typedef typename DofsArrayType::const_iterator DofConstantIteratorType;

    /// The definition of the DoF set type
    typedef typename std::unordered_set<TDofType::Pointer, DofPointerHasher> DofSetType;

    /// The containers of the entities
    typedef ModelPart::NodesContainerType NodesArrayType;
    typedef ModelPart::ElementsContainerType ElementsArrayType;
    typedef ModelPart::ConditionsContainerType ConditionsArrayType;

    /// The definition of the element container type
    typedef PointerVectorSet<Element, IndexedObject> ElementsContainerType;


    /// Pointer definition of ExplicitBuilderAndSolver
    KRATOS_CLASS_POINTER_DEFINITION(ExplicitBuilderAndSolver);

    ///@}
    ///@name Life Cycle
    ///@{

    /**
     * @brief Default constructor with Parameters
     * @param ThisParameters The configuration parameters
     */
    explicit ExplicitBuilderAndSolver(Parameters ThisParameters)
    {
        // Validate default parameters
        Parameters default_parameters = Parameters(R"(
        {
        })" );

        ThisParameters.ValidateAndAssignDefaults(default_parameters);
    }

    /**
     * @brief Default constructor.
     */
    explicit ExplicitBuilderAndSolver() = default;

    /** Destructor.
     */
    virtual ~ExplicitBuilderAndSolver() = default;


    ///@}
    ///@name Operators
    ///@{

    /**
     * @brief This method returns the flag mCalculateReactionsFlag
     * @return The flag that tells if the reactions are computed
     */
    bool GetCalculateReactionsFlag()
    {
        return mCalculateReactionsFlag;
    }

    /**
     * @brief This method sets the flag mCalculateReactionsFlag
     * @param CalculateReactionsFlag The flag that tells if the reactions are computed
     */
    void SetCalculateReactionsFlag(bool flag)
    {
        mCalculateReactionsFlag = flag;
    }

    /**
     * @brief This method returns the flag mDofSetIsInitialized
     * @return The flag that tells if the dof set is initialized
     */
    bool GetDofSetIsInitializedFlag()
    {
        return mDofSetIsInitialized;
    }

    /**
     * @brief This method sets the flag mDofSetIsInitialized
     * @param DofSetIsInitialized The flag that tells if the dof set is initialized
     */
    void SetDofSetIsInitializedFlag(bool DofSetIsInitialized)
    {
        mDofSetIsInitialized = DofSetIsInitialized;
    }

    /**
     * @brief This method returns the flag mReshapeMatrixFlag
     * @return The flag that tells if we need to reshape the LHS matrix
     */
    bool GetReshapeMatrixFlag()
    {
        return mReshapeMatrixFlag;
    }

    /**
     * @brief This method sets the flag mReshapeMatrixFlag
     * @param ReshapeMatrixFlag The flag that tells if we need to reshape the LHS matrix
     */
    void SetReshapeMatrixFlag(bool ReshapeMatrixFlag)
    {
        mReshapeMatrixFlag = ReshapeMatrixFlag;
    }

    /**
     * @brief This method returns the value mEquationSystemSize
     * @return Size of the system of equations
     */
    unsigned int GetEquationSystemSize()
    {
        return mEquationSystemSize;
    }

    // /**
    //  * @brief Function to perform the building of the LHS, depending on the implementation choosen the size of the matrix could be equal to the total number of Dofs or to the number unrestrained dofs
    //  * @param pScheme The pointer to the integration scheme
    //  * @param rModelPart The model part to compute
    //  * @param rA The LHS matrix of the system of equations
    //  */
    // virtual void BuildLHS(
    //     typename TSchemeType::Pointer pScheme,
    //     ModelPart& rModelPart,
    //     TSystemMatrixType& rA
    //     )
    // {
    // }

    /**
     * @brief Function to perform the build of the RHS. The vector could be sized as the total number of dofs or as the number of unrestrained ones
     * @param rModelPart The model part to compute
     * @param rb The RHS vector of the system of equations
     */
    virtual void BuildRHS(
        ModelPart& rModelPart,
        TSystemVectorType& rb)
    {
        KRATOS_TRY

        // Build the Right Hand Side without Dirichlet conditions
        // We skip setting the Dirichlet nodes residual to zero for the sake of efficiency
        // Note that this is not required as the Dirichlet conditions are set in the strategy
        BuildRHSNoDirichlet(rModelPart);

        KRATOS_CATCH("")
    }

    /**
     * @brief Function to perform the build of the RHS. The vector could be sized as the total number of dofs or as the number of unrestrained ones
     * @param rModelPart The model part to compute
     * @param rb The RHS vector of the system of equations
     */
    virtual void BuildRHSNoDirichlet(ModelPart& rModelPart)
    {
        KRATOS_TRY

        // Initialize the reaction (residual)
        InitializeDofSetReactions();

        // Gets the array of elements, conditions and constraints from the modeler
        const auto &r_elements_array = rModelPart.Elements();
        const auto &r_conditions_array = rModelPart.Conditions();
        const auto &r_constraints_array = rModelPart.MasterSlaveConstraints();
        const SizeType n_elems = static_cast<int>(r_elements_array.size());
        const SizeType n_conds = static_cast<int>(r_conditions_array.size());
        const SizeType n_constraints = static_cast<int>(r_constraints_array.size());

        const auto& r_process_info = rModelPart.GetProcessInfo();

#pragma omp parallel firstprivate(n_elems, n_conds, n_constraints)
        {
#pragma omp for schedule(guided, 512) nowait
            // Assemble all elements
            for (int i_elem = 0; i_elem < n_elems; ++i_elem) {
                auto it_elem = r_elements_array.begin() + i_elem;
                // Detect if the element is active or not. If the user did not make any choice the element is active by default
                // TODO: We will require to update this as soon as we remove the mIsDefined from the Flags
                bool element_is_active = true;
                if (it_elem->IsDefined(ACTIVE)) {
                    element_is_active = it_elem->Is(ACTIVE);
                }

                if (element_is_active) {
                    // Calculate elemental explicit residual contribution
                    // The explicit builder and solver assumes that the residual contribution is assembled in the REACTION variables
                    it_elem->AddExplicitContribution(r_process_info);
                }
            }

            // Assemble all conditions
#pragma omp for schedule(guided, 512)
            for (int i_cond = 0; i_cond < n_conds; ++i_cond) {
                auto it_cond = r_conditions_array.begin() + i_cond;
                // Detect if the condition is active or not. If the user did not make any choice the condition is active by default
                // TODO: We will require to update this as soon as we remove the mIsDefined from the Flags
                bool condition_is_active = true;
                if (it_cond->IsDefined(ACTIVE)) {
                    condition_is_active = it_cond->Is(ACTIVE);
                }

                if (condition_is_active) {
                    // Calculate condition explicit residual contribution
                    // The explicit builder and solver assumes that the residual contribution is assembled in the REACTION variables
                    it_cond->AddExplicitContribution(r_process_info);
                }
            }
        }

        KRATOS_CATCH("")
    }

    /**
     * @brief It applies the dirichlet conditions. This operation may be very heavy or completely unexpensive depending on the implementation choosen and on how the System Matrix is built.
     * @details For explanation of how it works for a particular implementation the user should refer to the particular Builder And Solver choosen
    //  * @param pScheme The pointer to the integration scheme
     * @param rModelPart The model part to compute
     * @param rA The LHS matrix of the system of equations
     * @param rDx The vector of unkowns
     * @param rb The RHS vector of the system of equations
     */
    virtual void ApplyDirichletConditions(
        ModelPart& rModelPart,
        TSystemMatrixType& rA,
        TSystemVectorType& rDx,
        TSystemVectorType& rb
        )
    {
    }

    /**
     * @brief The same of the precedent but affecting only the RHS
    //  * @param pScheme The pointer to the integration scheme
     * @param rModelPart The model part to compute
     * @param rA The LHS matrix of the system of equations
     * @param rb The RHS vector of the system of equations
     */
    virtual void ApplyDirichletConditions_RHS(
        ModelPart& rModelPart,
        TSystemVectorType& rDx,
        TSystemVectorType& rb
        )
    {
    }

    /**
     * @brief equivalent (but generally faster) then performing BuildLHS and BuildRHS
     * @param rModelPart The model part to compute
     * @param rA The LHS matrix of the system of equations
     * @param rb The RHS vector of the system of equations
     */
    virtual void Build(
        ModelPart &rModelPart,
        TSystemMatrixType &rA,
        TSystemVectorType &rb)
    {
    }

    /**
     * @brief Applies the constraints
    //  * @param pScheme The pointer to the integration scheme
     * @param rModelPart The model part to compute
     * @param rb The RHS vector of the system of equations
     */
    virtual void ApplyConstraints(
        ModelPart& rModelPart,
        TSystemMatrixType& rA,
        TSystemVectorType& rb
        )
    {
    }

    /**
     * @brief Builds the list of the DofSets involved in the problem by "asking" to each element and condition its Dofs.
     * @details The list of dofs is stores insde the ExplicitBuilderAndSolver as it is closely connected to the way the matrix and RHS are built
     * @param rModelPart The model part to compute
     */
    virtual void SetUpDofSet(const ModelPart& rModelPart)
    {
        KRATOS_TRY;

        KRATOS_INFO_IF("ExplicitBuilderAndSolver", ( this->GetEchoLevel() > 1 && rModelPart.GetCommunicator().MyPID() == 0)) << "Setting up the dofs" << std::endl;
        KRATOS_INFO_IF("ExplicitBuilderAndSolver", ( this->GetEchoLevel() > 2 && rModelPart.GetCommunicator().MyPID() == 0)) << "Number of threads" << OpenMPUtils::GetNumThreads() << "\n" << std::endl;

        // Gets the array of elements, conditions and constraints from the modeler
        const auto &r_elements_array = rModelPart.Elements();
        const auto &r_conditions_array = rModelPart.Conditions();
        const auto &r_constraints_array = rModelPart.MasterSlaveConstraints();
        const SizeType n_elems = static_cast<int>(r_elements_array.size());
        const SizeType n_conds = static_cast<int>(r_conditions_array.size());
        const SizeType n_constraints = static_cast<int>(r_constraints_array.size());

        // Global dof set
        DofSetType dof_global_set;
        dof_global_set.reserve(n_elems*20);

        // Auxiliary DOFs list
        DofsVectorType dof_list;
        DofsVectorType second_dof_list; // The second_dof_list is only used on constraints to include master/slave relations

#pragma omp parallel firstprivate(dof_list, second_dof_list)
        {
            const auto& r_process_info = rModelPart.GetProcessInfo();

            // We cleate the temporal set and we reserve some space on them
            DofSetType dofs_tmp_set;
            dofs_tmp_set.reserve(20000);

            // Get the DOFs list from each element and insert it in the temporary set
#pragma omp for schedule(guided, 512) nowait
            for (int i_elem = 0; i_elem < n_elems; ++i_elem) {
                const auto it_elem = r_elements_array.begin() + i_elem;
                it_elem->GetDofList(dof_list, r_process_info);
                dofs_tmp_set.insert(dof_list.begin(), dof_list.end());
            }

            // Get the DOFs list from each condition and insert it in the temporary set
#pragma omp for schedule(guided, 512) nowait
            for (int i_cond = 0; i_cond < n_conds; ++i_cond) {
                const auto it_cond = r_conditions_array.begin() + i_cond;
                it_cond->GetDofList(dof_list, r_process_info);
                dofs_tmp_set.insert(dof_list.begin(), dof_list.end());
            }

            // Get the DOFs list from each constraint and insert it in the temporary set
#pragma omp for  schedule(guided, 512) nowait
            for (int i_const = 0; i_const < n_constraints; ++i_const) {
                auto it_const = r_constraints_array.begin() + i_const;
                it_const->GetDofList(dof_list, second_dof_list, r_process_info);
                dofs_tmp_set.insert(dof_list.begin(), dof_list.end());
                dofs_tmp_set.insert(second_dof_list.begin(), second_dof_list.end());
            }

            // We merge all the sets in one thread
#pragma omp critical
            {
                dof_global_set.insert(dofs_tmp_set.begin(), dofs_tmp_set.end());
            }
        }

        KRATOS_INFO_IF("ExplicitBuilderAndSolver", ( this->GetEchoLevel() > 2)) << "Initializing ordered array filling\n" << std::endl;

        // Ordering the global DOF set
        mDofSet = DofsArrayType();
        DofsArrayType temp_dof_set;
        temp_dof_set.reserve(dof_global_set.size());
        for (auto it_dof = dof_global_set.begin(); it_dof != dof_global_set.end(); ++it_dof) {
            temp_dof_set.push_back(*it_dof);
        }
        temp_dof_set.Sort();
        mDofSet = temp_dof_set;

        // DoFs set checks
        // Throws an exception if there are no Degrees Of Freedom involved in the analysis
        KRATOS_ERROR_IF(mDofSet.size() == 0) << "No degrees of freedom!" << std::endl;

        // Check if each DOF has a reaction. Note that the explicit residual is stored in these
        for (auto it_dof = mDofSet.begin(); it_dof != mDofSet.end(); ++it_dof) {
            KRATOS_ERROR_IF_NOT(it_dof->HasReaction()) << "Reaction variable not set for the following : " <<std::endl
                << "Node : " << it_dof->Id() << std::endl
                << "Dof : " << (*it_dof) << std::endl << "Not possible to calculate reactions." << std::endl;
        }

        mDofSetIsInitialized = true;

        KRATOS_INFO_IF("ExplicitBuilderAndSolver", ( this->GetEchoLevel() > 2)) << "Number of degrees of freedom:" << mDofSet.size() << std::endl;
        KRATOS_INFO_IF("ExplicitBuilderAndSolver", ( this->GetEchoLevel() > 2 && rModelPart.GetCommunicator().MyPID() == 0)) << "Finished setting up the dofs" << std::endl;
        KRATOS_INFO_IF("ExplicitBuilderAndSolver", ( this->GetEchoLevel() > 2)) << "End of setup dof set\n" << std::endl;

        KRATOS_CATCH("");
    }

    virtual void SetUpDofSetEquationIds()
    {
        auto& r_dof_set = GetDofSet();
        const SizeType n_dofs = r_dof_set.size();

#pragma omp parallel for firstprivate(n_dofs)
        for (int i_dof = 0; i_dof < static_cast<int>(n_dofs); ++i_dof) {
            auto it_dof = r_dof_set.begin() + i_dof;
            it_dof->SetEquationId(i_dof);
        }
    }

    virtual void SetUpLumpedMassMatrixVector(const ModelPart &rModelPart)
    {
        KRATOS_TRY;

        KRATOS_INFO_IF("ExplicitBuilderAndSolver", ( this->GetEchoLevel() > 1 && rModelPart.GetCommunicator().MyPID() == 0)) << "Setting up the lumped mass matrix vector" << std::endl;
        KRATOS_INFO_IF("ExplicitBuilderAndSolver", ( this->GetEchoLevel() > 2 && rModelPart.GetCommunicator().MyPID() == 0)) << "Number of threads" << OpenMPUtils::GetNumThreads() << "\n" << std::endl;

        // Initialize the lumped mass matrix vector
        // Note that the lumped mass matrix vector size matches the dof set one
        mpLumpedMassMatrixVector = TSystemVectorPointerType(new TSystemVectorType(GetDofSet().size()));
        TDenseSpace::SetToZero(*mpLumpedMassMatrixVector);

        // Loop the elements to get the mass matrix
        LocalSystemVectorType elem_mass_vector;
        LocalSystemMatrixType elem_mass_matrix;
        const auto &r_elements_array = rModelPart.Elements();
        const auto &r_process_info = rModelPart.GetProcessInfo();
        const SizeType n_elems = static_cast<int>(r_elements_array.size());

#pragma omp for private(elem_mass_vector, elem_mass_matrix) schedule(guided, 512) nowait
        for (int i_elem = 0; i_elem < n_elems; ++i_elem) {
            const auto it_elem = r_elements_array.begin() + i_elem;
            auto& r_geom = it_elem->GetGeometry();

            // Calculate the elemental lumped mass vector
            it_elem->CalculateMassMatrix(elem_mass_matrix, r_process_info);
            const SizeType n_dofs_elem = elem_mass_matrix.size2();
            if (elem_mass_vector.size() != n_dofs_elem) {
                TDenseSpace::Resize(elem_mass_vector, n_dofs_elem);
            }
            for (IndexType i = 0; i < elem_mass_matrix.size1(); ++i) {
                elem_mass_vector(i) = 0.0;
                for (IndexType j = 0; j < elem_mass_matrix.size2(); ++j) {
                    elem_mass_vector(i) += elem_mass_matrix(i,j);
                }
            }

            // Set it in the global lumped mass vector
            const SizeType n_nodes = r_geom.PointsNumber();
            for (IndexType i_node = 0; i_node < n_nodes; ++i_node) {
                const auto& r_node_dofs = r_geom[i_node].GetDofs();
                const SizeType n_dofs = r_node_dofs.size();
                for (int i_dof = 0; i_dof < static_cast<int>(n_dofs); ++i_dof) {
                    const SizeType eq_id = r_node_dofs[i_dof]->EquationId();
                    const double aux_mass = elem_mass_vector(i_node * n_dofs + i_dof);
// NOTE THAT THIS IS A BOTTLENECK--> WE ASSUME THAT WE WILL NOT USE THE SPACES IN HERE
// #pragma omp critical
//                     {
//                     const double mass_value = TDenseSpace::GetValue(*mpLumpedMassMatrixVector, eq_id);
//                     TDenseSpace::SetValue(*mpLumpedMassMatrixVector, eq_id, aux_mass + mass_value);
//                     }
#pragma omp atomic
                    (*mpLumpedMassMatrixVector)[eq_id] += aux_mass;
                }
            }
        }

        KRATOS_CATCH("");
    }

    /**
     * @brief It allows to get the list of Dofs from the element
     */
    virtual DofsArrayType& GetDofSet()
    {
        return mDofSet;
    }

    /**
     * @brief Get the lumped mass matrix vector pointer
     * It allows to get the lumped mass matrix vector pointer
     * @return TSystemVectorPointerType& The lumped mass matrix vector pointer
     */
    virtual TSystemVectorPointerType& pGetLumpedMassMatrixVector()
    {
        return mpLumpedMassMatrixVector;
    }

    /**
     * @brief Get the lumped mass matrix vector
     * It allows to get the lumped mass matrix vector
     * @return TSystemVectorType& The lumped mass matrix vector
     */
    virtual TSystemVectorType& GetLumpedMassMatrixVector()
    {
        return (*mpLumpedMassMatrixVector);
    }

    /**
     * @brief Initialize the DOF set reactions
     * For an already initialized dof set (mDofSet), this method sets to
     * zero the corresponding reaction variable values. Note that in the
     * explicit build the reactions are used as residual container.
     */
    virtual void InitializeDofSetReactions()
    {
        KRATOS_ERROR_IF_NOT(mDofSetIsInitialized) << "Trying to initialize the explicit residual but the DOFs set is not initialized yet." << std::endl;

#pragma parallel for
        for (int i_dof = 0; i_dof < mDofSet.size(); ++i_dof) {
            auto it_dof = mDofSet.begin() + i_dof;
            auto& r_reaction_value = it_dof->GetSolutionStepReactionValue();
            r_reaction_value = 0.0;
        }
    }

    /**
     * @brief It applies certain operations at the system of equations at the begining of the solution step
     * @param rModelPart The model part to compute
     */
    virtual void InitializeSolutionStep(ModelPart& rModelPart)
    {
    }

    /**
     * @brief It applies certain operations at the system of equations at the end of the solution step
     * @param rModelPart The model part to compute
     * @param rA The LHS matrix of the system of equations
     * @param rDx The vector of unkowns
     * @param rb The RHS vector of the system of equations
     */
    virtual void FinalizeSolutionStep(ModelPart& rModelPart)
    {
    }

    /**
     * @brief It computes the reactions of the system
     * @param rModelPart The model part to compute
     */
    virtual void CalculateReactions(ModelPart& rModelPart)
    {
        // TODO
        // THIS IS MINUS THE RHS
    }

    /**
     * @brief This function is intended to be called at the end of the solution step to clean up memory
    storage not needed
     */
    virtual void Clear()
    {
        this->mDofSet = DofsArrayType();
        this->mpReactionsVector.reset();

        KRATOS_INFO_IF("ExplicitBuilderAndSolver", this->GetEchoLevel() > 0) << "Clear Function called" << std::endl;
    }

    /**
     * @brief This function is designed to be called once to perform all the checks needed
     * on the input provided. Checks can be "expensive" as the function is designed
     * to catch user's errors.
     * @param rModelPart The model part to compute
     * @return 0 all ok
     */
    virtual int Check(ModelPart& rModelPart)
    {
        KRATOS_TRY

        return 0;

        KRATOS_CATCH("");
    }

    /**
     * @brief It sets the level of echo for the solving strategy
     * @param Level The level to set
     * @details The different levels of echo are:
     * - 0: Mute... no echo at all
     * - 1: Printing time and basic informations
     * - 2: Printing linear solver data
     * - 3: Print of debug informations: Echo of stiffness matrix, Dx, b...
     * - 4: Print of stiffness matrix, b to Matrix Market
     */
    void SetEchoLevel(int Level)
    {
        mEchoLevel = Level;
    }

    /**
     * @brief It returns the echo level
     * @return The echo level of the builder and solver
     */
    int GetEchoLevel()
    {
        return mEchoLevel;
    }

    ///@}
    ///@name Operations
    ///@{

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    virtual std::string Info() const
    {
        return "ExplicitBuilderAndSolver";
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const
    {
        rOStream << Info();
    }

    /// Print object's data.
    virtual void PrintData(std::ostream& rOStream) const
    {
        rOStream << Info();
    }

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

    DofsArrayType mDofSet; /// The set containing the DoF of the system

    TSystemVectorPointerType mpLumpedMassMatrixVector;

    bool mReshapeMatrixFlag = false;  /// If the matrix is reshaped each step

    bool mDofSetIsInitialized = false; /// Flag taking care if the dof set was initialized ot not

    bool mCalculateReactionsFlag = false; /// Flag taking in account if it is needed or not to calculate the reactions

    unsigned int mEquationSystemSize; /// Number of degrees of freedom of the problem to be solve

    int mEchoLevel = 0;

    TSystemVectorPointerType mpReactionsVector;

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
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
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


    ///@}

}; /* Class ExplicitBuilderAndSolver */

///@}

///@name Type Definitions
///@{


///@}

} /* namespace Kratos.*/

#endif /* KRATOS_EXPLICIT_BUILDER_AND_SOLVER  defined */
