from __future__ import print_function, absolute_import, division

# import kratos
import KratosMultiphysics as Kratos

# import formulation interface
from KratosMultiphysics.RANSApplication.formulations.rans_formulation import RansFormulation

# import formulations
from ..incompressible_potential_flow import IncompressiblePotentialFlowRansFormulation
from ..k_omega import KOmegaRansFormulation
from .fractional_step_velocity_pressure_rans_formulation import FractionalStepVelocityPressureRansFormulation

class FractionalStepKOmegaRansFormulation(RansFormulation):
    def __init__(self, model_part, settings):
        super().__init__(model_part, settings)

        default_settings = Kratos.Parameters(r'''
        {
            "formulation_name": "fractional_step_k_epsilon",
            "incompressible_potential_flow_initialization_settings": {},
            "fractional_step_flow_solver_settings": {},
            "k_omega_solver_settings": {},
            "max_iterations": 1
        }''')
        settings.ValidateAndAssignDefaults(default_settings)

        if (not settings["incompressible_potential_flow_initialization_settings"].IsEquivalentTo(
                Kratos.Parameters("{}"))):
            self.incompressible_potential_flow_formulation = IncompressiblePotentialFlowRansFormulation(model_part, settings["incompressible_potential_flow_initialization_settings"])
            self.AddRansFormulation(self.incompressible_potential_flow_formulation)

        self.fractional_step_formulation = FractionalStepVelocityPressureRansFormulation(model_part, settings["fractional_step_flow_solver_settings"])
        self.AddRansFormulation(self.fractional_step_formulation)

        self.k_omega_formulation = KOmegaRansFormulation(model_part, settings["k_omega_solver_settings"])
        self.AddRansFormulation(self.k_omega_formulation)

        self.SetMaxCouplingIterations(settings["max_iterations"].GetInt())

    def SetConstants(self, settings):
        self.k_omega_formulation.SetConstants(settings)