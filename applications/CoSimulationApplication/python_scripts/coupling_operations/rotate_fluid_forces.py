from __future__ import print_function, absolute_import, division  # makes these scripts backward compatible with python 2.6 and 2.7

# Importing the Kratos Library
import KratosMultiphysics as KM
import KratosMultiphysics.ChimeraApplication as ChimeraApp

import numpy as np
import math

# Importing the base class
from KratosMultiphysics.CoSimulationApplication.base_classes.co_simulation_coupling_operation import CoSimulationCouplingOperation

# CoSimulation imports
import KratosMultiphysics.CoSimulationApplication.co_simulation_tools as cs_tools

def Create(settings, solver_wrappers):
    cs_tools.SettingsTypeCheck(settings)
    return RotateFluidForcesOperation(settings, solver_wrappers)

class RotateFluidForcesOperation(CoSimulationCouplingOperation):
    """This operation Rotates the fluid forces from the deformed config
        to the original config. The rotation angle is obtained from the
        modelpart itself. ChimeraApp's Rotate region process calculates
        and puts it on the modelpart under variable ChimeraApp.ROTATIONAL_ANGLE

        This operation is intended to be used together with the ChimeraApp
        for FSI simulation of rotating bodies (wind turbines)

        IMPORTANT: Requires scipy and numpy. Works with compiled KratosMultiphysics

        Since fluid forces are reactios and are not used internally in FluidDynamicaApp
        we replace the actual REACTION with rotated ones
    """
    def __init__(self, settings, solver_wrappers):
        super(RotateFluidForcesOperation, self).__init__(settings)
        solver_name = self.settings["solver"].GetString()
        data_name = self.settings["data_name"].GetString()
        self.interface_data = solver_wrappers[solver_name].GetInterfaceData(data_name)

    def Initialize(self):
        pass

    def Finalize(self):
        pass

    def InitializeSolutionStep(self):
        pass

    def FinalizeSolutionStep(self):
        pass

    def InitializeCouplingIteration(self):
        pass

    def FinalizeCouplingIteration(self):
        pass

    def Execute(self):
        self.model_part = self.interface_data.GetModelPart()
        # -1*angle  because we have to rotate the forces BACK on to un rotated config
        angle_of_rotation = -1*self.model_part.GetValue(ChimeraApp.ROTATIONAL_ANGLE)
        axis_of_rotation = self.settings["axis_of_rotation"].GetVector()
        print("RotateFluidForcesOperation: ROTATIONAL_ANGLE : ", angle_of_rotation)
        print("RotateFluidForcesOperation: ROTATIONAL_AXIS  : ", axis_of_rotation)
        for node in self.model_part.Nodes:
            data_vector = node.GetSolutionStepValue(self.interface_data.variable)
            rotated_data = self.__RotateVector(data_vector,angle_of_rotation, axis_of_rotation)
            rotated_data_vector = KM.Vector(3)
            rotated_data_vector[0] = rotated_data[0]
            rotated_data_vector[1] = rotated_data[1]
            rotated_data_vector[2] = rotated_data[2]
            node.SetSolutionStepValue(self.interface_data.variable, 0, rotated_data_vector)

    def PrintInfo(self):
        pass

    def Check(self):
        pass

    @classmethod
    def _GetDefaultSettings(cls):
        this_defaults = KM.Parameters("""{
            "solver"    : "UNSPECIFIED",
            "data_name" : "UNSPECIFIED",
            "axis_of_rotation" : []
        }""")
        this_defaults.AddMissingParameters(super(RotateFluidForcesOperation, cls)._GetDefaultSettings())
        return this_defaults

    def __RotateVector(self,vector, theta, axis):
        """
        Following the answer in :
        https://stackoverflow.com/questions/6802577/rotation-of-3d-vector
        """
        M = self.__GetRotationMatrix(theta, axis)
        return np.dot(M,vector)

    def __GetRotationMatrix(self, theta, axis):
        """
        Return the rotation matrix associated with counterclockwise rotation about
        the given axis by theta radians.
        """
        axis = np.asarray(axis)
        axis = axis / math.sqrt(np.dot(axis, axis))
        a = math.cos(theta / 2.0)
        b, c, d = -axis * math.sin(theta / 2.0)
        aa, bb, cc, dd = a * a, b * b, c * c, d * d
        bc, ad, ac, ab, bd, cd = b * c, a * d, a * c, a * b, b * d, c * d
        return np.array([[aa + bb - cc - dd, 2 * (bc + ad), 2 * (bd - ac)],
                        [2 * (bc - ad), aa + cc - bb - dd, 2 * (cd + ab)],
                        [2 * (bd + ac), 2 * (cd - ab), aa + dd - bb - cc]])

