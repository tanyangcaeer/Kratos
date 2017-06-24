from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7
import KratosMultiphysics
import KratosMultiphysics.StructuralMechanicsApplication as StructuralMechanicsApplication
import structural_mechanics_solver

# Check that KratosMultiphysics was imported in the main script
KratosMultiphysics.CheckForPreviousImport()


def CreateSolver(main_model_part, custom_settings):
    return ImplicitMechanicalSolver(main_model_part, custom_settings)


class ImplicitMechanicalSolver(structural_mechanics_solver.MechanicalSolver):
    """The structural mechanics implicit dynamic solver.

    This class creates the mechanical solvers for implicit dynamic analysis.
    It supports Newmark, Bossak and dynamic relaxation schemes.

    Member variables from base class:
    settings -- Kratos parameters containing general solver settings.
    main_model_part -- the model part used to construct the solver.

    Additional member variables:
    dynamic_settings -- settings for the implicit dynamic solvers.
    """
    def __init__(self, main_model_part, custom_settings): 
        settings = custom_settings.Clone()
        self.dynamic_settings = KratosMultiphysics.Parameters("{}")
        if settings.Has("damp_factor_m"):
            self.dynamic_settings.AddValue("damp_factor_m", settings["damp_factor_m"])
            settings.RemoveValue("damp_factor_m")
        if not settings.Has("scheme_type"): # Assign the default dynamic scheme.
            settings.AddEmptyValue("scheme_type")
            settings["scheme_type"].SetString("Newmark")

        # Construct the base solver.
        super().__init__(main_model_part, settings)

        # Set defaults and validate custom settings.
        default_dynamic_settings = KratosMultiphysics.Parameters("""
        {
            "damp_factor_m" : -0.01
        }
        """)
        self.dynamic_settings.ValidateAndAssignDefaults(default_dynamic_settings)
        print("::[ImplicitMechanicalSolver]:: Construction finished")

    def Initialize(self):
        print("::[ImplicitMechanicalSolver]:: Initializing ...")
        # The solver is created here.
        super().Initialize()
        print("::[ImplicitMechanicalSolver]:: Finished initialization.")

    def AddVariables(self):
        super().AddVariables()
        if self.settings["rotation_dofs"].GetBool():
            self.main_model_part.AddNodalSolutionStepVariable(StructuralMechanicsApplication.POINT_TORQUE)
        print("::[ImplicitMechanicalSolver]:: Variables ADDED")
    
    #### Private functions ####

    def _create_solution_scheme(self):
        scheme_type = self.settings["scheme_type"].GetString()
        if(scheme_type == "Newmark"):
            damp_factor_m = 0.0
            mechanical_scheme = KratosMultiphysics.ResidualBasedBossakDisplacementScheme(damp_factor_m)
        elif(scheme_type == "Bossak"):
            damp_factor_m = self.dynamic_settings["damp_factor_m"].GetDouble()
            mechanical_scheme = KratosMultiphysics.ResidualBasedBossakDisplacementScheme(damp_factor_m)
        elif(scheme_type == "Relaxation"):
            damp_factor_f =-0.3
            dynamic_factor_m = 10.0
            mechanical_scheme = StructuralMechanicsApplication.ResidualBasedRelaxationScheme(
                                                                       damp_factor_f, dynamic_factor_m)
        else:
            raise Exception("Unsupported scheme_type: " + scheme_type)
        return mechanical_scheme

    def _create_mechanical_solver(self):
        computing_model_part = self.GetComputingModelPart()
        mechanical_scheme = self.get_solution_scheme()
        linear_solver = self.get_linear_solver()
        mechanical_convergence_criterion = self.get_convergence_criterion()
        builder_and_solver = self.get_builder_and_solver()
        if self.settings["line_search"].GetBool():
            mechanical_solver = self._create_line_search_strategy()
        else:
            mechanical_solver = self._create_newton_raphson_strategy()
        return mechanical_solver

    def _create_line_search_strategy(self):
        computing_model_part = self.GetComputingModelPart()
        mechanical_scheme = self.get_solution_scheme()
        linear_solver = self.get_linear_solver()
        mechanical_convergence_criterion = self.get_convergence_criterion()
        builder_and_solver = self.get_builder_and_solver()
        return KratosMultiphysics.LineSearchStrategy(computing_model_part, 
                                                     mechanical_scheme, 
                                                     linear_solver, 
                                                     mechanical_convergence_criterion, 
                                                     builder_and_solver, 
                                                     self.settings["max_iteration"].GetInt(),
                                                     self.settings["compute_reactions"].GetBool(),
                                                     self.settings["reform_dofs_at_each_step"].GetBool(),
                                                     self.settings["move_mesh_flag"].GetBool())

    def _create_newton_raphson_strategy(self):
        computing_model_part = self.GetComputingModelPart()
        mechanical_scheme = self.get_solution_scheme()
        linear_solver = self.get_linear_solver()
        mechanical_convergence_criterion = self.get_convergence_criterion()
        builder_and_solver = self.get_builder_and_solver()
        return KratosMultiphysics.ResidualBasedNewtonRaphsonStrategy(computing_model_part, 
                                                                     mechanical_scheme, 
                                                                     linear_solver, 
                                                                     mechanical_convergence_criterion, 
                                                                     builder_and_solver,
                                                                     self.settings["max_iteration"].GetInt(),
                                                                     self.settings["compute_reactions"].GetBool(),
                                                                     self.settings["reform_dofs_at_each_step"].GetBool(),
                                                                     self.settings["move_mesh_flag"].GetBool())
