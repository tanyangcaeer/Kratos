from __future__ import print_function, absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7
# importing the Kratos Library
import KratosMultiphysics
import KratosMultiphysics.CompressiblePotentialFlowApplication as KCPFApp
from KratosMultiphysics.CompressiblePotentialFlowApplication.potential_flow_solver import PotentialFlowSolver
from KratosMultiphysics.CompressiblePotentialFlowApplication.potential_flow_solver import PotentialFlowFormulation

class PotentialFlowAdjointFormulation(PotentialFlowFormulation):
    def _SetUpIncompressibleElement(self, formulation_settings):
        default_settings = KratosMultiphysics.Parameters(r"""{
            "element_type": "incompressible"
        }""")
        formulation_settings.ValidateAndAssignDefaults(default_settings)

        self.element_name = "AdjointIncompressiblePotentialFlowElement"
        self.condition_name = "AdjointPotentialWallCondition"

    def _SetUpCompressibleElement(self, formulation_settings):
        default_settings = KratosMultiphysics.Parameters(r"""{
            "element_type": "compressible"
        }""")
        formulation_settings.ValidateAndAssignDefaults(default_settings)

        self.element_name = "AdjointCompressiblePotentialFlowElement"
        self.condition_name = "AdjointPotentialWallCondition"

    def _SetUpEmbeddedIncompressibleElement(self, formulation_settings):
        raise RuntimeError("Adjoint embedded element currently not implemented")


def CreateSolver(model, custom_settings):
    return PotentialFlowAdjointSolver(model, custom_settings)

class PotentialFlowAdjointSolver(PotentialFlowSolver):
    def __init__(self, model, custom_settings):

        self.response_function_settings = custom_settings["response_function_settings"].Clone()
        self.sensitivity_settings = custom_settings["sensitivity_settings"].Clone()
        custom_settings.RemoveValue("response_function_settings")
        custom_settings.RemoveValue("sensitivity_settings")
        # Construct the base solver.
        super(PotentialFlowAdjointSolver, self).__init__(model, custom_settings)

        self.formulation = PotentialFlowAdjointFormulation(self.settings["formulation"])
        self.element_name = self.formulation.element_name
        self.condition_name = self.formulation.condition_name

        KratosMultiphysics.Logger.PrintInfo("::[PotentialFlowAdjointSolver]:: ", "Construction finished")

    def AddVariables(self):
        super(PotentialFlowAdjointSolver, self).AddVariables()
        self.main_model_part.AddNodalSolutionStepVariable(KCPFApp.ADJOINT_VELOCITY_POTENTIAL)
        self.main_model_part.AddNodalSolutionStepVariable(KCPFApp.ADJOINT_AUXILIARY_VELOCITY_POTENTIAL)
        self.main_model_part.AddNodalSolutionStepVariable(KratosMultiphysics.SHAPE_SENSITIVITY)

        KratosMultiphysics.Logger.PrintInfo("::[PotentialFlowAdjointSolver]:: ", "Variables ADDED")

    def AddDofs(self):
        KratosMultiphysics.VariableUtils().AddDof(KCPFApp.ADJOINT_VELOCITY_POTENTIAL, self.main_model_part)
        KratosMultiphysics.VariableUtils().AddDof(KCPFApp.ADJOINT_AUXILIARY_VELOCITY_POTENTIAL, self.main_model_part)

    def Initialize(self):
        """Perform initialization after adding nodal variables and dofs to the main model part. """
        if self.response_function_settings["response_type"].GetString() == "adjoint_lift_jump_coordinates":
            self.response_function = KCPFApp.AdjointLiftJumpCoordinatesResponseFunction(self.main_model_part, self.response_function_settings)
        else:
            raise Exception("invalid response_type: " + self.response_function_settings["response_type"].GetString())

        self.sensitivity_builder=KratosMultiphysics.SensitivityBuilder(self.sensitivity_settings,self.main_model_part, self.response_function)
        self.sensitivity_builder.Initialize()

        scheme = KratosMultiphysics.ResidualBasedAdjointStaticScheme(self.response_function)

        builder_and_solver = KratosMultiphysics.ResidualBasedBlockBuilderAndSolver(self.linear_solver)
        self.solver = KratosMultiphysics.ResidualBasedLinearStrategy(
            self.main_model_part,
            scheme,
            self.linear_solver,
            builder_and_solver,
            self.settings["compute_reactions"].GetBool(),
            self.settings["reform_dofs_at_each_step"].GetBool(),
            self.settings["calculate_solution_norm"].GetBool(),
            self.settings["move_mesh_flag"].GetBool())

        self.solver.SetEchoLevel(self.settings["echo_level"].GetInt())
        self.solver.Check()

        self.response_function.Initialize()

        KratosMultiphysics.Logger.PrintInfo("::[PotentialFlowAdjointSolver]:: ", "Finished initialization.")

    def PrepareModelPart(self):
        super(PotentialFlowAdjointSolver, self).PrepareModelPart()
        # defines how the primal elements should be replaced with their adjoint counterparts
        if self.response_function_settings["gradient_mode"].GetString()=="semi_analytic":
            replacement_settings = KratosMultiphysics.Parameters("""
            {
                "element_name" : "AdjointIncompressiblePotentialFlowElement2D3N",
                "condition_name" : "AdjointPotentialWallCondition2D2N"
            }
            """)
        elif self.response_function_settings["gradient_mode"].GetString()=="analytic":
            replacement_settings = KratosMultiphysics.Parameters("""
            {
                "element_name" : "AdjointAnalyticalIncompressiblePotentialFlowElement2D3N",
                "condition_name" : "AdjointPotentialWallCondition2D2N"
            }
            """)

        KratosMultiphysics.ReplaceElementsAndConditionsProcess(self.main_model_part, replacement_settings).Execute()

        KratosMultiphysics.Logger.PrintInfo("::[PotentialFlowAdjointSolver]:: ", "ModelPart prepared for Solver.")

    def InitializeSolutionStep(self):
        super(PotentialFlowAdjointSolver, self).InitializeSolutionStep()
        self.response_function.InitializeSolutionStep()

    def FinalizeSolutionStep(self):
        super(PotentialFlowAdjointSolver, self).FinalizeSolutionStep()
        self.response_function.FinalizeSolutionStep()
        self.sensitivity_builder.UpdateSensitivities()

