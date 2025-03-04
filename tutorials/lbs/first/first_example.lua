--[[ @doc

# A First Example

This is a complete simulation transport example. Each aspect of the simulation process is kept to a minimum:
- We use an orthogonal 2D grid;
- We introduce the concept of domain decomposition ("partitioning");
- The domain is homogeneous (single material, uniform isotropic external source), vacuum boundary conditions apply;
- The cross sections are given in a text file (with our OpenSn format); we use only one energy group in this example;
- The angular quadrature (discretization in angle) is introduced;
- The Linear Boltzmann Solver (LBS) options are keep to a minimum.


Because transport simulations are computationally expensive due to the high dimensional of the phase-space
(physical space, energy, direction), they are often performed using several parallel processes (e.g., CPU cores).
In what follows, we enforce that this first example be run with 4 MPI processes.

---
## Check the number of processes
This portion of the lua input is not mandatory. The user is free to delete it and run the code with a different number
of processes. However, to reproduce the graphics below, one needs to run with 4 MPI ranks.

The lua input performs this following checks:
+ `check_num_procs==nil` will be true when running serially without MPI
+ `number_of_processes ~= num_procs` will be true when the number of MPI processes requested is not equal to the preset value of `4`.

To run the code, simply type: `mpiexec -n 4 path/to/opensn  input_filename.lua`

For more runtime options, type `path/to/opensn -h` for help.
--]]
-- Check num_procs
num_procs = 4
if (check_num_procs==nil and number_of_processes ~= num_procs) then
  Log(LOG_0ERROR,"Incorrect amount of processors. " ..
    "Expected "..tostring(num_procs)..
    ". Pass check_num_procs=false to override if possible.")
  os.exit(false)
end

--[[ @doc
## Mesh
Here, we will use the in-house orthogonal mesh generator for a simple Cartesian grid.
### List of nodes
We first create a lua table for the list of nodes. The nodes will be spread from -1 to +1.
Be mindful that lua indexing starts at 1.
--]]
-- Setup the mesh
nodes={}
n_cells=10
length=2.
xmin = -length/2.
dx = length/n_cells
for i=1,(n_cells+1) do
    k=i-1
    nodes[i] = xmin + k*dx
end
--[[ @doc
### Orthogonal Mesh Generation
We use the `OrthogonalMeshGenerator` and pass the list of nodes per dimension. Here, we pass 2 times the same list of
nodes to create a 2D geometry with square cells. Thus, we create a square domain, of side length 2, centered on the origin (0,0).

We also partition the 2D mesh into 2x2 subdomains using `KBAGraphPartitioner`. Since we want the split the x-axis in 2,
we give only 1 value in the xcuts array (x=0). Likewise for ycuts (y=0). The assignment to a partition is done based on where the
cell center is located with respect to the various xcuts, ycuts, and zcuts (a fuzzy logic is applied to avoid issues).

The resulting mesh and partition is shown below:

![Mesh_Partition](images/first_example_mesh_partition.png)
--]]
meshgen = mesh.OrthogonalMeshGenerator.Create
({
  node_sets = {nodes,nodes},
  partitioner = KBAGraphPartitioner.Create
  ({
    nx = 2, ny=2,
    xcuts = {0.0}, ycuts = {0.0},
  })
})

mesh.MeshGenerator.Execute(meshgen)

--[[ @doc
### Material IDs
We create a right parallelepiped logical volume that contains the entire mesh and we assign a 0 for material ID
to all cells found inside the logical volume. Logical volumes are quite powerful, see subsequent tutorials on
post-processing.
--]]
-- Set Material IDs
vol0 = mesh.RPPLogicalVolume.Create({infx=true, infy=true, infz=true})
mesh.SetMaterialIDFromLogicalVolume(vol0,0)

--[[ @doc
## Materials
We create a material and add two properties to it:
+ TRANSPORT_XSECTIONS for the transport cross sections, and
+ ISOTROPIC_MG_SOURCE for the isotropic volumetric source
--]]
-- Add materials
materials = {}
materials[1] = PhysicsAddMaterial("Material_A");

PhysicsMaterialAddProperty(materials[1],TRANSPORT_XSECTIONS)
PhysicsMaterialAddProperty(materials[1],ISOTROPIC_MG_SOURCE)

--[[ @doc
## Cross Sections
We assign the cross sections to the material by loading the file containing the cross sections. See the tutorials'
section on materials for more details on cross sections.
--]]
PhysicsMaterialSetProperty(materials[1],TRANSPORT_XSECTIONS,
  OPENSN_XSFILE,"xs_1g_MatA.xs")

--[[ @doc
## Volumetric Source
We create a lua table containing the volumetric multigroup source and assign it to the material by passing that array.
--]]
num_groups = 1
src={}
for g=1,num_groups do
  src[g] = 1.0
end
PhysicsMaterialSetProperty(materials[1],ISOTROPIC_MG_SOURCE,FROM_ARRAY,src)

--[[ @doc
## Angular Quadrature
We create a product Gauss-Legendre-Chebyshev angular quadrature and pass the number of **positive** polar cosines
(here `npolar = 2`) and the number of azimuthal subdivisions in **one quadrant** (`nazimu = 1`).
This creates a 3D angular quadrature.

We finish by optimizing the quadrature to only use the positive hemisphere for 2D simulations.
--]]
-- Setup the Angular Quadrature
nazimu = 1
npolar = 2
pquad = CreateProductQuadrature(GAUSS_LEGENDRE_CHEBYSHEV,nazimu,npolar)
OptimizeAngularQuadratureForPolarSymmetry(pquad, 4.0*math.pi)

--[[ @doc
## Linear Boltzmann Solver
### Options for the Linear Boltzmann Solver (LBS)
In the LBS block, we provide
+ the number of energy groups,
+ the groupsets (with 0-indexing), the handle for the angular quadrature, the angle aggregation, the solver type,
tolerances, and other solver options.
--]]
-- Setup LBS parameters
lbs_block =
{
  num_groups = num_groups,
  groupsets =
  {
    {
      groups_from_to = {0, 0},
      angular_quadrature_handle = pquad,
      angle_aggregation_num_subsets = 1,
      inner_linear_method = "gmres",
      l_abs_tol = 1.0e-6,
      l_max_its = 300,
      gmres_restart_interval = 30,
    }
  }
}
--[[ @doc
### Putting the Linear Boltzmann Solver Together
We then create the physics solver, initialize it, and execute it.
--]]
phys = lbs.DiscreteOrdinatesSolver.Create(lbs_block)

-- Initialize and Execute Solver
ss_solver = lbs.SteadyStateSolver.Create({lbs_solver_handle = phys})

SolverInitialize(ss_solver)
SolverExecute(ss_solver)

--[[ @doc
## Post-Processing via Field Functions
We extract the scalar flux (i.e., the first entry in the field function list; recall that lua
indexing starts at 1) and export it to a VTK file whose name is supplied by the user. See the tutorials' section
on post-processing for more details on field functions.

The resulting scalar flux is shown below:

![Scalar_flux](images/first_example_scalar_flux.png)
--]]
-- Retrieve field functions and export them
fflist,count = LBSGetScalarFieldFunctionList(phys)
vtk_basename = "first_example"
ExportFieldFunctionToVTK(fflist[1],vtk_basename)

--[[ @doc
## Possible Extensions
1. Change the number of MPI processes (you may want to delete the safeguard at the top of the input file to run with any number of MPI ranks);
2. Change the spatial resolution by increasing or decreasing the number of cells;
3. Change the angular resolution by increasing or decreasing the number of polar and azimuthal subdivisions.
--]]