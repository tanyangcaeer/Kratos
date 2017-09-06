from __future__ import print_function, absolute_import, division
import KratosMultiphysics 

import KratosMultiphysics.StructuralMechanicsApplication as StructuralMechanicsApplication
import KratosMultiphysics.KratosUnittest as KratosUnittest

from math import sqrt, sin, cos, pi, exp, atan

class TestCrBeam3D2N(KratosUnittest.TestCase):
    def setUp(self):
        pass

    def _add_dofs(self,mp):
        for node in mp.Nodes:
            node.AddDof(KratosMultiphysics.DISPLACEMENT_X)
            node.AddDof(KratosMultiphysics.DISPLACEMENT_Y)
            node.AddDof(KratosMultiphysics.DISPLACEMENT_Z)
            node.AddDof(KratosMultiphysics.REACTION_X)
            node.AddDof(KratosMultiphysics.REACTION_Y)
            node.AddDof(KratosMultiphysics.REACTION_Z)
            node.AddDof(KratosMultiphysics.ROTATION_X)
            node.AddDof(KratosMultiphysics.ROTATION_Y)
            node.AddDof(KratosMultiphysics.ROTATION_Z)

            
    
    def _add_variables(self,mp):
        mp.AddNodalSolutionStepVariable(KratosMultiphysics.DISPLACEMENT)
        mp.AddNodalSolutionStepVariable(KratosMultiphysics.REACTION)
        mp.AddNodalSolutionStepVariable(KratosMultiphysics.ROTATION)
        mp.AddNodalSolutionStepVariable(StructuralMechanicsApplication.POINT_LOAD)  
        mp.AddNodalSolutionStepVariable(StructuralMechanicsApplication.POINT_MOMENT)  
        mp.AddNodalSolutionStepVariable(KratosMultiphysics.VOLUME_ACCELERATION) 
        mp.AddNodalSolutionStepVariable(KratosMultiphysics.VELOCITY)
        mp.AddNodalSolutionStepVariable(KratosMultiphysics.ACCELERATION)     
        mp.AddNodalSolutionStepVariable(KratosMultiphysics.ANGULAR_VELOCITY)
        mp.AddNodalSolutionStepVariable(KratosMultiphysics.ANGULAR_ACCELERATION)      


        
    def _apply_material_properties(self,mp,dim):
        #define properties
        mp.GetProperties()[0].SetValue(KratosMultiphysics.YOUNG_MODULUS,210e9)
        mp.GetProperties()[0].SetValue(KratosMultiphysics.DENSITY,7850)
        mp.GetProperties()[0].SetValue(StructuralMechanicsApplication.CROSS_AREA,0.01)
        mp.GetProperties()[0].SetValue(KratosMultiphysics.POISSON_RATIO,0.30)
        local_inertia_vector = KratosMultiphysics.Vector(2)
        local_inertia_vector[0] = 0.00001
        local_inertia_vector[1] = 0.00001
        #local_inertia_vector[2] = 0.00001

        mp.GetProperties()[0].SetValue(StructuralMechanicsApplication.LOCAL_INERTIA_VECTOR,local_inertia_vector)

        g = [0,0,0]  
        mp.GetProperties()[0].SetValue(KratosMultiphysics.VOLUME_ACCELERATION,g)

        cl = StructuralMechanicsApplication.LinearElastic3DLaw()
        mp.GetProperties()[0].SetValue(KratosMultiphysics.CONSTITUTIVE_LAW,cl)

        
    def _apply_BCs(self,mp,which_dof):
        
        if (which_dof == 'xyz'):
            for node in mp.Nodes:
                node.Fix(KratosMultiphysics.DISPLACEMENT_X)
                node.Fix(KratosMultiphysics.DISPLACEMENT_Y)
                node.Fix(KratosMultiphysics.DISPLACEMENT_Z)
        if (which_dof == 'xz'):
            for node in mp.Nodes:
                node.Fix(KratosMultiphysics.DISPLACEMENT_X)
                node.Fix(KratosMultiphysics.DISPLACEMENT_Z)
        if (which_dof == 'yz'):
            for node in mp.Nodes:
                node.Fix(KratosMultiphysics.DISPLACEMENT_Y)
                node.Fix(KratosMultiphysics.DISPLACEMENT_Z)
        if (which_dof == 'rotXYZ'):
            for node in mp.Nodes:
                node.Fix(KratosMultiphysics.ROTATION_X)
                node.Fix(KratosMultiphysics.ROTATION_Y)
                node.Fix(KratosMultiphysics.ROTATION_Z)                

    def _apply_Neumann_BCs(self,mp,which_dof,load_size_dir):

        if(which_dof == 'y'):
            for node in mp.Nodes:
                node.SetSolutionStepValue(StructuralMechanicsApplication.
                POINT_LOAD_Y,0,load_size_dir)        
        if(which_dof == 'x'):
            for node in mp.Nodes:
                node.SetSolutionStepValue(StructuralMechanicsApplication.
                POINT_LOAD_X,0,load_size_dir)  

        
    def _solve_linear(self,mp):
        linear_solver = KratosMultiphysics.SkylineLUFactorizationSolver()
        builder_and_solver = KratosMultiphysics.ResidualBasedBlockBuilderAndSolver(linear_solver)
        scheme = KratosMultiphysics.ResidualBasedIncrementalUpdateStaticScheme()
        
        compute_reactions = True
        reform_step_dofs = True
        calculate_norm_dx = False
        move_mesh_flag = True
        strategy = KratosMultiphysics.ResidualBasedLinearStrategy(mp, 
                                                                scheme, 
                                                                linear_solver, 
                                                                builder_and_solver, 
                                                                compute_reactions, 
                                                                reform_step_dofs, 
                                                                calculate_norm_dx,
                                                                move_mesh_flag)
        strategy.SetEchoLevel(0)
        
        strategy.Check()
        strategy.Solve()

    def _solve_nonlinear(self,mp):
        linear_solver = KratosMultiphysics.SkylineLUFactorizationSolver()
        builder_and_solver = KratosMultiphysics.ResidualBasedBlockBuilderAndSolver(linear_solver)
        scheme = KratosMultiphysics.ResidualBasedIncrementalUpdateStaticScheme()
        convergence_criterion = StructuralMechanicsApplication.DisplacementAndOtherDoFCriteria(1e-8,1e-8)
        convergence_criterion.SetEchoLevel(0)
        
        max_iters = 1000
        compute_reactions = True
        reform_step_dofs = True
        move_mesh_flag = True
        strategy = KratosMultiphysics.ResidualBasedNewtonRaphsonStrategy(mp, 
                                                                scheme, 
                                                                linear_solver, 
                                                                convergence_criterion,
                                                                builder_and_solver, 
                                                                max_iters,
                                                                compute_reactions, 
                                                                reform_step_dofs, 
                                                                move_mesh_flag)
        strategy.SetEchoLevel(0)
        
        strategy.Check()
        strategy.Solve()
    def _solve_dynamic(self,mp):
        
        #define a minimal newton raphson solver
        linear_solver = KratosMultiphysics.SkylineLUFactorizationSolver()
        builder_and_solver = KratosMultiphysics.ResidualBasedBlockBuilderAndSolver(linear_solver)
        scheme = KratosMultiphysics.ResidualBasedBossakDisplacementScheme(0.00)
        convergence_criterion = KratosMultiphysics.ResidualCriteria(1e-8,1e-8)
        convergence_criterion.SetEchoLevel(0)


        max_iters = 1000
        compute_reactions = True
        reform_step_dofs = True
        move_mesh_flag = True
        strategy = KratosMultiphysics.ResidualBasedNewtonRaphsonStrategy(mp, 
                                                                scheme, 
                                                                linear_solver, 
                                                                convergence_criterion,
                                                                builder_and_solver, 
                                                                max_iters,
                                                                compute_reactions, 
                                                                reform_step_dofs, 
                                                                move_mesh_flag)
        strategy.SetEchoLevel(0)
        
        strategy.Check()
        strategy.Solve()
        
           
    
    def _check_results_linear(self,mp):
        
        #1.) check displacement result
        displacement_nodes = [mp.Nodes[1].GetSolutionStepValue(
            KratosMultiphysics.DISPLACEMENT),mp.Nodes[2].GetSolutionStepValue(
            KratosMultiphysics.DISPLACEMENT),mp.Nodes[3].GetSolutionStepValue(
            KratosMultiphysics.DISPLACEMENT)]
        for i in range(3):
            k = 0.00
            if (i == 1): k = -0.0026619856874997507
            self.assertAlmostEqual(displacement_nodes[1][i], k)
            self.assertAlmostEqual(displacement_nodes[0][i], 0)
            self.assertAlmostEqual(displacement_nodes[2][i], 0)

        #2.) check reactions
        reaction_nodes = [mp.Nodes[1].GetSolutionStepValue(
            KratosMultiphysics.REACTION),mp.Nodes[2].GetSolutionStepValue(
            KratosMultiphysics.REACTION),mp.Nodes[3].GetSolutionStepValue(
            KratosMultiphysics.REACTION)]

        Force_y = 1000000
        self.assertAlmostEqual(reaction_nodes[0][0], Force_y)
        self.assertAlmostEqual(reaction_nodes[2][0], -Force_y)
        self.assertAlmostEqual(reaction_nodes[0][1], Force_y/2)
        self.assertAlmostEqual(reaction_nodes[2][1], Force_y/2)
    def _check_results_nonlinear(self,mp,timestep,Force_i):
        ##node1
        node_temp = mp.Nodes[1]
        disp_temp = node_temp.GetSolutionStepValue(KratosMultiphysics.DISPLACEMENT)
        reac_temp = node_temp.GetSolutionStepValue(KratosMultiphysics.REACTION)
        #disp_y
        self.assertAlmostEqual(disp_temp[1], 0)
        #reaction_y
        reaction_y_node1 = Force_i*(-1)
        self.assertAlmostEqual(reac_temp[1],reaction_y_node1,6)
        #reaction_x 
        reaction_x_node1 = [741464.9276510741,1485888.977636112,2233316.9009164227,
        2983794.615716884,3737369.2509119534,4494089.191516033,5254004.126397622,
        6017165.0983619075,6783624.556737246,7553436.412628534,8326656.0969987875,
        9103340.621764094,9883548.644087134,10667340.53408764,11454778.446184492,
        12245926.394319195,13040850.331311818,13839618.232645638,14642300.18497437,
        15448968.47968232,16259697.711865114,17074564.885111626,17893649.522510014,
        18717033.784337185,19544802.592936598,20377043.765315644,21213848.154068694,
        22055309.79726978,22901526.07803619,23752597.894554257,24608629.841397412,
        25469730.40309285,26336012.160943132,27207592.014241144,28084591.41712631,
        28967136.632448073,29855359.004160944,30749395.24993747,31649387.775854316,
        32555485.015236698,33467841.79396308,34386619.72480394,35311987.633672595,
        36244122.02100001,37183207.5618443,38129437.64879564,39083014.9822317,
        40044152.21308944,41013072.64397469,41990010.99523344,42975214.24348978,
        43968942.54123137,44971470.22722923,45983086.93901924,47004098.840344116,
        48034829.97843962,49075623.78836585,50126844.76436112,51188880.321473464,
        52262142.87466787,53347072.16731208,54444137.88663925,55553842.61067612,
        56676725.13951358,57813364.2740804,58964383.118212394,60130453.99549516,
        61312304.09186774,62510721.95949238,63726565.048343465,64960768.47141639,
        66214355.26005781,67488448.43149039,68784285.27627705,70103234.38657254,
        71446816.09699601,72816727.21376368,74214871.18642414,75643395.26295514,
        77104736.71277626,78601680.98043492,80137435.76669273,81715726.72014935,
        83340922.98755075,85018204.87282823,86753792.28015186,88555263.27634439,
        90432010.47500862,92395916.01811945,94462388.67855237,96652033.38549507,
        98993500.26523624,101528726.98334643,104323616.16359182,107493197.43582612,
        111276440.23647068,116390127.39236663,-62782528.388332605,-63351316.30823133,
        -63919034.598836,-64485690.945303164,-65051292.93836311,-65615848.075949684,
        -66179363.76479947,-66741847.3220098,-67303305.9765681,-67863746.8708426,
        -68423177.06204486,-68981603.5236578,-69539033.1468354,-70095472.74176757,
        -70650929.0390236,-71205408.69085957,-71758918.27250087,-72311464.28340018,
        -72863053.1484657,-73413691.21926463,-73963384.77520159,-74512140.02467461,
        -75059963.10620539,]
        self.assertAlmostEqual(reac_temp[0],reaction_x_node1[timestep])

        ##node2
        node_temp = mp.Nodes[2]
        disp_temp = node_temp.GetSolutionStepValue(KratosMultiphysics.DISPLACEMENT_Y)
        reac_temp = node_temp.GetSolutionStepValue(KratosMultiphysics.REACTION)
        load_temp = node_temp.GetSolutionStepValue(StructuralMechanicsApplication.POINT_LOAD_Y)
        #pointLoad
        self.assertAlmostEqual(load_temp,Force_i)
        #reaction_x
        self.assertAlmostEqual(reac_temp[0],reaction_x_node1[timestep]*(-1))
        #displacement_y
        EA = 210e9*0.01
        L = sqrt(4+1)
        L3 = L*L*L
        P_i = ((EA/(2*L3))*(disp_temp*disp_temp +2*1*disp_temp)*(disp_temp+1))
        self.assertAlmostEqual(P_i,Force_i,1)


    def _check_results_dynamic(self,mp,time_i):
        
        #analaytical free-vibration node 3
        we1 = 7917.25
        we2 = 19113.94
        y1 = 1.4142*2.874e-5
        y2 = -1.4142*4.93107e-6
        test_disp_temp = y1*(1-cos(we1*time_i))-y2*(1-cos(we2*time_i))
        simulated_disp_temp = mp.Nodes[3].GetSolutionStepValue(
            KratosMultiphysics.DISPLACEMENT_X)  

        self.assertAlmostEqual(simulated_disp_temp, test_disp_temp,6)

        #analaytical free-vibration node 2
        we1 = 7917.25
        we2 = 19113.94
        y1 = 1.000*2.874e-5
        y2 = 1.000*4.93107e-6
        test_disp_temp = y1*(1-cos(we1*time_i))-y2*(1-cos(we2*time_i))
        simulated_disp_temp = mp.Nodes[2].GetSolutionStepValue(
            KratosMultiphysics.DISPLACEMENT_X)  

        self.assertAlmostEqual(simulated_disp_temp, test_disp_temp,6)
    def _set_and_fill_buffer(self,mp,buffer_size,delta_time):
        # Set buffer size
        mp.SetBufferSize(buffer_size)

        # Fill buffer
        time = mp.ProcessInfo[KratosMultiphysics.TIME]
        time = time - delta_time * (buffer_size)
        mp.ProcessInfo.SetValue(KratosMultiphysics.TIME, time)
        for size in range(0, buffer_size):
            step = size - (buffer_size -1)
            mp.ProcessInfo.SetValue(KratosMultiphysics.STEP, step)
            time = time + delta_time
            #delta_time is computed from previous time in process_info
            mp.CloneTimeStep(time)

        mp.ProcessInfo[KratosMultiphysics.IS_RESTARTED] = False        

    def test_cr_beam_linear(self):
        dim = 3
        nr_nodes = 11
        nr_elements = nr_nodes-1
        mp = KratosMultiphysics.ModelPart("solid_part")
        self._add_variables(mp)
        self._apply_material_properties(mp,dim)

        #create nodes
        for i in range(nr_nodes):
            dx = 0.1
            mp.CreateNewNode(i+1,i*dx,0.00,0.00)
        #add dofs
        self._add_dofs(mp)
        #create condition
        mp.CreateNewCondition("PointLoadCondition3D1N",1,[11],mp.GetProperties()[0])  
        #create submodelparts for dirichlet boundary conditions
        bcs_xyz = mp.CreateSubModelPart("Dirichlet_XYZ")
        bcs_xyz.AddNodes([1])
        bcs_rot = mp.CreateSubModelPart("Dirichlet_RotAll")
        bcs_rot.AddNodes([1])    
        #create a submodalpart for neumann boundary conditions
        bcs_neumann = mp.CreateSubModelPart("PointLoad3D_neumann")
        bcs_neumann.AddNodes([11])
        bcs_neumann.AddConditions([1])             
        #create Element
        for i in range(nr_elements):
            mp.CreateNewElement("CrLinearBeamElement3D2N", i+1, [i+1,i+2],
             mp.GetProperties()[0])

        #apply boundary conditions
        Force_Y = -400000.00
        self._apply_BCs(bcs_xyz,'xyz')
        self._apply_BCs(bcs_rot,'rotXYZ')
        self._apply_Neumann_BCs(bcs_neumann,'y',Force_Y)

        
        #solve + compare
        self._solve_nonlinear(mp)    
        #self._check_results_linear(mp)


    def test_cr_beam_nonlinear(self):
        dim = 3
        mp = KratosMultiphysics.ModelPart("solid_part")
        self._add_variables(mp)
        self._apply_material_properties(mp,dim)

        #create nodes
        mp.CreateNewNode(1,0.0,0.0,0.0)
        mp.CreateNewNode(2,0.5,0.0,0.0)
        mp.CreateNewNode(3,1.0,0.0,0.0)
        #add dofs
        self._add_dofs(mp)
        
    
    def test_cr_beam_dynamic(self):
        dim = 3
        mp = KratosMultiphysics.ModelPart("solid_part")
        self._add_variables(mp)
        self._apply_material_properties(mp,dim)

        #create nodes
        mp.CreateNewNode(1,0.0,0.0,0.0)
        mp.CreateNewNode(2,0.5,0.0,0.0)
        mp.CreateNewNode(3,1.0,0.0,0.0)
        #add dofs
        self._add_dofs(mp)

        
if __name__ == '__main__':
    KratosUnittest.main()

