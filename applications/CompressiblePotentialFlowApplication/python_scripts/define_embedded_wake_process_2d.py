import KratosMultiphysics
import KratosMultiphysics.CompressiblePotentialFlowApplication as CPFApp
import time
import math


def Factory(settings, Model):
    if( not isinstance(settings,KratosMultiphysics.Parameters) ):
        raise Exception("expected input shall be a Parameters object, encapsulating a json string")

    return DefineEmbeddedWakeProcess(Model, settings["Parameters"])


class DefineEmbeddedWakeProcess(KratosMultiphysics.Process):
    def __init__(self, Model, settings):
        # Call the base Kratos process constructor
        KratosMultiphysics.Process.__init__(self)

        # Check default settings
        default_settings = KratosMultiphysics.Parameters(r'''{
            "model_part_name": "",
            "epsilon": 1e-9
        }''')
        settings.ValidateAndAssignDefaults(default_settings)

        self.main_model_part = Model[settings["model_part_name"].GetString()].GetRootModelPart()
        self.wake_model_part=Model.CreateModelPart("wake")

        self.epsilon = settings["epsilon"].GetDouble()

    def ExecuteInitializeSolutionStep(self):
        ini_time = time.time()


        self.__FindWakeElements()
        max_inactive_x = -1e30
        for elem in self.main_model_part.Elements:
            if elem.IsNot(KratosMultiphysics.ACTIVE):
                max_inactive_x = max(elem.GetGeometry().Center().X, max_inactive_x)
        lower_surface_elem_ids=[]
        for elem in self.main_model_part.Elements:
            if elem.Is(KratosMultiphysics.TO_SPLIT) and elem.Is(KratosMultiphysics.ACTIVE):
                for node in elem.GetNodes():
                    distance = node.GetSolutionStepValue(CPFApp.GEOMETRY_DISTANCE)
                    if distance > 0.0:
                        node_pos = node
                    else:
                        node_neg = node

                if node_pos.Y - self.trailing_edge_node.Y> 0.0:
                    for node in elem.GetNodes():
                        node.SetValue(CPFApp.UPPER_SURFACE, True)
                else:
                    lower_surface_elem_ids.append(elem.Id)
                    for node in elem.GetNodes():
                        node.SetValue(CPFApp.LOWER_SURFACE, True)

        if self.main_model_part.HasSubModelPart("lower_surface_sub_model_part"):
            self.main_model_part.RemoveSubModelPart("lower_surface_sub_model_part")
        self.lower_surface_element_sub_model_part = self.main_model_part.CreateSubModelPart("lower_surface_sub_model_part")
        self.lower_surface_element_sub_model_part.AddElements(lower_surface_elem_ids)

        for node in self.main_model_part.Nodes:
            if node.GetValue(CPFApp.UPPER_SURFACE) and node.GetValue(CPFApp.LOWER_SURFACE) and node.X < self.trailing_edge_node.X  and node.X > max_inactive_x-0.1:
                node.SetValue(CPFApp.TRAILING_EDGE, True)
                print(node.Id, end=' ')

        for elem in self.lower_surface_element_sub_model_part.Elements:
            if not elem.GetValue(CPFApp.WAKE) and not elem.GetValue(CPFApp.KUTTA) and elem.Is(KratosMultiphysics.ACTIVE):
                for node in elem.GetNodes():
                    if node.GetValue(CPFApp.TRAILING_EDGE):
                        elem.SetValue(CPFApp.KUTTA, True)
        self.trailing_edge_node.SetSolutionStepValue(CPFApp.GEOMETRY_DISTANCE, 1e-9)



        KratosMultiphysics.Logger.PrintInfo('EmbeddedWake','Wake computation time: ',time.time()-ini_time)

    def _DefineWakeModelPart(self):
        ''' This function generates the modelpart of the wake. TODO: make end of the domain user-definable.
        '''
        self.wake_model_part.CreateNewNode(1, 0.0, 0.0, 0.0)
        self.wake_model_part.CreateNewNode(2, 200.0, 0.0, 0.0)
        self.wake_model_part.CreateNewElement("Element2D2N", 1, [1,2], KratosMultiphysics.Properties(0))

    def _MoveAndRotateWake(self):
        ''' This function moves and rotates the wake with the same parameters as the geometry.
        '''
        self.moving_parameters = KratosMultiphysics.Parameters()
        self.moving_parameters.AddEmptyValue("origin")
        # self.moving_parameters["origin"].SetVector(self.main_model_part.ProcessInfo.GetValue(CPFApp.WAKE_ORIGIN))
        # self.moving_parameters.AddEmptyValue("rotation_angle")
        # angle=math.radians(-self.main_model_part.ProcessInfo.GetValue(CPFApp.ROTATION_ANGLE))
        # self.moving_parameters["rotation_angle"].SetDouble(angle)


        max_x = -1e30
        for element in self.main_model_part.Elements:
            if element.IsNot(KratosMultiphysics.ACTIVE):
                x_center = element.GetGeometry().Center().X
                if x_center > max_x:
                    max_x = x_center
                    max_y = element.GetGeometry().Center().Y
        wake_origin = KratosMultiphysics.Vector(3, 0.0)
        wake_origin[0] = max_x - 0.00
        wake_origin[1] = max_y

        self.moving_parameters["origin"].SetVector(wake_origin)
        self.moving_parameters.AddEmptyValue("rotation_angle")
        self.moving_parameters["rotation_angle"].SetDouble(0.0)


        CPFApp.MoveModelPartProcess(self.wake_model_part, self.moving_parameters).Execute()

    def ExecuteFinalizeSolutionStep(self):
        # self.wake_sub_model_part = self.main_model_part.CreateSubModelPart("wake_sub_model_part")
        # for elem in self.main_model_part.Elements:
        #     if elem.GetValue(CPFApp.WAKE) and elem.Is(KratosMultiphysics.ACTIVE):
        #         self.wake_sub_model_part.Elements.append(elem)

        absolute_tolerance = 1e-6
        CPFApp.PotentialFlowUtilities.CheckIfWakeConditionsAreFulfilled2D(self.wake_sub_model_part, absolute_tolerance, 2)
        self.main_model_part.RemoveSubModelPart("wake_sub_model_part")


    def __FindWakeElements(self):

        if not self.main_model_part.HasSubModelPart("trailing_edge_sub_model_part"):
            self.trailing_edge_sub_model_part = self.main_model_part.CreateSubModelPart("trailing_edge_sub_model_part")
        else: self.trailing_edge_sub_model_part = self.main_model_part.GetSubModelPart("trailing_edge_sub_model_part")
        if not self.main_model_part.HasSubModelPart("wake_sub_model_part"):
            self.wake_sub_model_part = self.main_model_part.CreateSubModelPart("wake_sub_model_part")
        else: self.wake_sub_model_part = self.main_model_part.GetSubModelPart("wake_sub_model_part")
        #List to store trailing edge elements id and wake elements id
        self.trailing_edge_element_id_list = []
        self.wake_element_id_list = []

        self.__SetWakeDirectionAndNormal()
        # Save the trailing edge for further computations
        self.__SaveTrailingEdgeNode()
        self.trailing_edge_sub_model_part.AddNodes([self.trailing_edge_node.Id])

        # Check which elements are cut and mark them as wake
        self.__MarkWakeElements()
        # Mark the elements touching the trailing edge from below as kutta
        self.__MarkKuttaElements()
        # Mark the trailing edge element that is further downstream as wake
        self.__MarkWakeTEElement()

    def __SetWakeDirectionAndNormal(self):
        free_stream_velocity = self.main_model_part.ProcessInfo.GetValue(CPFApp.FREE_STREAM_VELOCITY)
        if(free_stream_velocity.Size() != 3):
            raise Exception('The free stream velocity should be a vector with 3 components!')
        self.wake_direction = KratosMultiphysics.Vector(3)
        vnorm = math.sqrt(
            free_stream_velocity[0]**2 + free_stream_velocity[1]**2 + free_stream_velocity[2]**2)
        self.wake_direction[0] = free_stream_velocity[0]/vnorm
        self.wake_direction[1] = free_stream_velocity[1]/vnorm
        self.wake_direction[2] = free_stream_velocity[2]/vnorm

        self.wake_normal = KratosMultiphysics.Vector(3)
        self.wake_normal[0] = -self.wake_direction[1]
        self.wake_normal[1] = self.wake_direction[0]
        self.wake_normal[2] = 0.0

    def __SaveTrailingEdgeNode(self):
        # This function finds and saves the trailing edge for further computations
        max_x_coordinate = -1e30
        for elem in self.main_model_part.Elements:
            if elem.Is(KratosMultiphysics.TO_SPLIT) and elem.Is(KratosMultiphysics.ACTIVE):
                for node in elem.GetNodes():
                    if(node.X > max_x_coordinate) and node.GetSolutionStepValue(CPFApp.GEOMETRY_DISTANCE) < 0.0:
                        max_x_coordinate = node.X
                        self.trailing_edge_node = node
        # self.trailing_edge_node.SetSolutionStepValue(CPFApp.GEOMETRY_DISTANCE, 10)
        # self.trailing_edge_node = self.main_model_part.GetNode(53640)
        self.trailing_edge_node.SetValue(CPFApp.TRAILING_EDGE, True)

    def __MarkWakeElements(self):
        # This function checks which elements are cut by the wake
        # and marks them as wake elements
        KratosMultiphysics.Logger.PrintInfo('...Selecting wake elements...')

        for elem in self.main_model_part.Elements:
            # Mark and save the elements touching the trailing edge
            self.__MarkTrailingEdgeElement(elem)

            # Elements downstream the trailing edge can be wake elements
            potentially_wake = self.__CheckIfPotentiallyWakeElement(elem)

            if(potentially_wake):
                # Compute the nodal distances of the element to the wake
                distances_to_wake = self.__ComputeDistancesToWake(elem)

                # Selecting the cut (wake) elements
                is_wake_element = self.__CheckIfWakeElement(distances_to_wake)

                if(is_wake_element):
                    elem.SetValue(CPFApp.WAKE, True)
                    elem.SetValue(CPFApp.WAKE_ELEMENTAL_DISTANCES, distances_to_wake)
                    counter=0
                    self.wake_element_id_list.append(elem.Id)
                    for node in elem.GetNodes():
                        node.SetValue(CPFApp.WAKE_DISTANCE,distances_to_wake[counter])
                        counter += 1
        self.wake_sub_model_part.AddElements(self.wake_element_id_list)
        self.__SaveTrailingEdgeElements()

        KratosMultiphysics.Logger.PrintInfo('...Selecting wake elements finished...')

    def __MarkTrailingEdgeElement(self, elem):
        # This function marks the elements touching the trailing
        # edge and saves them in the trailing_edge_element_id_list
        # for further computations
        for elnode in elem.GetNodes():
            if(elnode.GetValue(CPFApp.TRAILING_EDGE)):
                elem.SetValue(CPFApp.TRAILING_EDGE, True)
                self.trailing_edge_element_id_list.append(elem.Id)
                break

    def __SaveTrailingEdgeElements(self):
        # This function stores the trailing edge element
        # to its submodelpart.
        self.trailing_edge_sub_model_part.AddElements(self.trailing_edge_element_id_list)

    def __CheckIfPotentiallyWakeElement(self, elem):
        # This function selects the elements downstream the
        # trailing edge as potentially wake elements

        # Compute the distance from the element's center to
        # the trailing edge
        x_distance_to_te = elem.GetGeometry().Center().X - self.trailing_edge_node.X
        y_distance_to_te = elem.GetGeometry().Center().Y - self.trailing_edge_node.Y

        # Compute the projection of the distance in the wake direction
        projection_on_wake = x_distance_to_te*self.wake_direction[0] + \
            y_distance_to_te*self.wake_direction[1]

        # Elements downstream the trailing edge can be wake elements
        if(projection_on_wake > 0):
            return True
        else:
            return False

    def __ComputeDistancesToWake(self, elem):
        # This function computes the distance of the element nodes
        # to the wake
        nodal_distances_to_wake = KratosMultiphysics.Vector(3)
        counter = 0
        for elnode in elem.GetNodes():
            # Compute the distance from the node to the trailing edge
            x_distance_to_te = elnode.X - self.trailing_edge_node.X
            y_distance_to_te = elnode.Y - self.trailing_edge_node.Y

            # Compute the projection of the distance vector in the wake normal direction
            distance_to_wake = x_distance_to_te*self.wake_normal[0] + \
                y_distance_to_te*self.wake_normal[1]

            # Nodes laying on the wake have a positive distance
            if(abs(distance_to_wake) < self.epsilon):
                distance_to_wake = self.epsilon

            nodal_distances_to_wake[counter] = distance_to_wake
            counter += 1

        return nodal_distances_to_wake

    @staticmethod
    def __CheckIfWakeElement(distances_to_wake):
        # This function checks whether the element is cut by the wake

        # Initialize counters
        number_of_nodes_with_positive_distance = 0
        number_of_nodes_with_negative_distance = 0

        # Count how many element nodes are above and below the wake
        for nodal_distance_to_wake in distances_to_wake:
            if(nodal_distance_to_wake < 0.0):
                number_of_nodes_with_negative_distance += 1
            else:
                number_of_nodes_with_positive_distance += 1

        # Elements with nodes above and below the wake are wake elements
        return(number_of_nodes_with_negative_distance > 0 and number_of_nodes_with_positive_distance > 0)

    def __MarkKuttaElements(self):
        # This function selects the kutta elements. Kutta elements
        # are touching the trailing edge from below.
        for elem in self.trailing_edge_sub_model_part.Elements:
            # Compute the distance from the element center to the trailing edge
            x_distance_to_te = elem.GetGeometry().Center().X - self.trailing_edge_node.X
            y_distance_to_te = elem.GetGeometry().Center().Y - self.trailing_edge_node.Y

            # Compute the projection of the distance vector in the wake normal direction
            distance_to_wake = x_distance_to_te*self.wake_normal[0] + \
                y_distance_to_te*self.wake_normal[1]

            # Marking the elements under the trailing edge as kutta
            if(distance_to_wake < 0.0):
                elem.SetValue(CPFApp.KUTTA, True)

    @staticmethod
    def __CheckIfElemIsCutByWake(elem):
        nneg=0
        # REMINDER: In 3D the elemental_distances may not be match with
        # the nodal distances if CalculateDistanceToSkinProcess is used
        distances = elem.GetValue(CPFApp.WAKE_ELEMENTAL_DISTANCES)
        for nodal_distance in distances:
            if nodal_distance<0:
                nneg += 1

        return nneg==1

    def __MarkWakeTEElement(self):
        # This function finds the trailing edge element that is further downstream
        # and marks it as wake trailing edge element. The rest of trailing edge elements are
        # unassigned from the wake.

        for elem in self.trailing_edge_sub_model_part.Elements:
            if (elem.GetValue(CPFApp.WAKE)):
                if(self.__CheckIfElemIsCutByWake(elem)): #TE Element
                    elem.Set(KratosMultiphysics.STRUCTURE)
                    elem.SetValue(CPFApp.KUTTA, False)
                else: #Rest of elements touching the trailing edge but not part of the wake
                    elem.SetValue(CPFApp.WAKE, False)
                    self.wake_sub_model_part.RemoveElement(elem)

    def _CleanMarking(self):
        # This function removes all the markers set by _FindWakeElements()
        for elem in self.trailing_edge_sub_model_part.Elements:
            elem.SetValue(CPFApp.TRAILING_EDGE, False)
            elem.Reset(KratosMultiphysics.STRUCTURE)
            elem.SetValue(CPFApp.KUTTA, False)

        for elem in self.wake_sub_model_part.Elements:
            elem.SetValue(CPFApp.WAKE, False)
            elem.Set(KratosMultiphysics.TO_ERASE, True)
        self.wake_sub_model_part.RemoveElements(KratosMultiphysics.TO_ERASE)

        self.trailing_edge_element_id_list = []
        self.wake_element_id_list = []
