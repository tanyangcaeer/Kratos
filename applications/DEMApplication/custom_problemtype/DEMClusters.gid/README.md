
# Cluster Generator
Available for __Windows__ and __Linux__.

Based on the copyrighted __SphereTree Toolkit__ found [here](http://isg.cs.tcd.ie/spheretree/).

Permission to use, copy, modify and distribute this software and its
  documentation for educational, research and non-profit purposes, without
  fee, and without a written agreement is hereby granted, provided that the
  above copyright notice and the following paragraphs appear in all copies.

The __SphereTree Toolkit__ authors may be contacted at the following e-mail addresses:
- Gareth_Bradshaw@yahoo.co.uk
- isg@cs.tcd.ie



# Set up
Once downloaded and added as problemtype, it is required to put the required executables in the exec folder.
- For Windows the precompiled executables can be found [here](http://isg.cs.tcd.ie/spheretree/downloads/spheretree-1.0-win32.zip)
- For Linux, the spheretree source code must be compiled.

# Use
Once the geometry has been generated and meshed, the user can choose between step-by-step process or automatically generated the final cluster file.

The step-by-step process go as follows:
- Click on calculate in order to generate the OBJ and MSH files
- Define the options for the spheretree algorithms or use the default options (recommended)
- Select generate SPH (see examples for execution time references)
- Select generate CLU
- Select visualize cluster over mesh. The cluster will always be approximately centered at the origin.

# Examples
Both examples are created on a Intel i7 laptop.

- Jar, 6000 tetrahedra
branch: 20, time: 2 min
branch: 200, time: 14 min

<span>
<img align="center" src="https://github.com/KratosMultiphysics/Examples/raw/master/fluid_dynamics/use_cases/barcelona_wind/resources/BarcelonaVelocityVector.png" width="288">
  Jar
</span>
<br>


- Candy, 268 tetrahedra
branch: 20, time:1 min
branch: 200, time: 7 min

<span>
<img align="center" src="https://github.com/KratosMultiphysics/Examples/raw/master/fluid_dynamics/use_cases/barcelona_wind/resources/BarcelonaVelocityVector.png" width="288">
  Candy
</span>
<br>






# Recommendations and troubleshooting
In order to avoid typical issues when generating the cluster.
- Do not generate the geometry from an existing mesh
- If using a copy of an existing geometry via save as, save and reload the problem before continuing.
- The mesh is not automatically generated. The user must specify the mesh size and generate the mesh prior to create the SPH file or the cluster.

