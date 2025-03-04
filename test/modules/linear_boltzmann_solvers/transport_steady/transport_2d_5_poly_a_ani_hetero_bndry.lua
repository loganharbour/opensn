-- 2D Transport test with Vacuum and Incident-isotropic BC.
-- SDM: PWLD
-- Test: Max-value1=3.18785
num_procs = 4





--############################################### Check num_procs
if (check_num_procs==nil and number_of_processes ~= num_procs) then
  Log(LOG_0ERROR,"Incorrect amount of processors. " ..
    "Expected "..tostring(num_procs)..
    ". Pass check_num_procs=false to override if possible.")
  os.exit(false)
end

--############################################### Setup mesh
nodes={}
N=40
L=10.0
xmin = -L/2
dx = L/N
for i=1,(N+1) do
  k=i-1
  nodes[i] = xmin + k*dx
end

meshgen1 = mesh.OrthogonalMeshGenerator.Create({ node_sets = {nodes,nodes} })
mesh.MeshGenerator.Execute(meshgen1)

--############################################### Set Material IDs
mesh.SetUniformMaterialID(0)
--############################################### Add materials
materials = {}
materials[1] = PhysicsAddMaterial("Test Material");

PhysicsMaterialAddProperty(materials[1],TRANSPORT_XSECTIONS)

PhysicsMaterialAddProperty(materials[1],ISOTROPIC_MG_SOURCE)


num_groups = 1
PhysicsMaterialSetProperty(materials[1],TRANSPORT_XSECTIONS,
  OPENSN_XSFILE,"xs_air50RH.xs")

src={}
for g=1,num_groups do
  src[g] = 0.0
end
--src[1] = 1.0
PhysicsMaterialSetProperty(materials[1],ISOTROPIC_MG_SOURCE,FROM_ARRAY,src)

--############################################### Setup Physics
pquad0 = CreateProductQuadrature(GAUSS_LEGENDRE_CHEBYSHEV,12, 2)
OptimizeAngularQuadratureForPolarSymmetry(pquad0, 4.0*math.pi)

lbs_block =
{
  num_groups = num_groups,
  groupsets =
  {
    {
      groups_from_to = {0, 0},
      angular_quadrature_handle = pquad0,
      angle_aggregation_num_subsets = 1,
      groupset_num_subsets = 2,
      inner_linear_method = "gmres",
      l_abs_tol = 1.0e-6,
      l_max_its = 300,
      gmres_restart_interval = 100,
    },
  }
}

--int cell_global_id
--int material_id

--VecXYZ location (.x .y and .z)
--VecXYZ normal

--array<int>      quadrature_angle_indices
--array<VecXYZ>   quadrature_angle_vectors
--array<PhiTheta> quadrature_phi_theta_angles (PhiTheta.phi and PhiTheta.theta)
--array<int>      group_indices

--double          evaluation_time
function luaBoundaryFunctionA(cell_global_id,
                              material_id,
                              location,
                              normal,
                              quadrature_angle_indices,
                              quadrature_angle_vectors,
                              quadrature_phi_theta_angles,
                              group_indices,
                              time)
    num_angles = rawlen(quadrature_angle_vectors)
    num_groups = rawlen(group_indices)
    psi = {}
    dof_count = 0

    for ni=1,num_angles do
        omega = quadrature_angle_vectors[ni]
        phi_theta = quadrature_phi_theta_angles[ni]
        for gi=1,num_groups do
            g = group_indices[gi]

            value = 1.0
            if (location.y < 0.0 or omega.y < 0.0) then
                value = 0.0
            end

            dof_count = dof_count + 1
            psi[dof_count] = value
        end
    end

    return psi
end

lbs_options =
{
  boundary_conditions =
  {
    {
      name = "xmin",
      type = "incident_anisotropic_heterogeneous",
      function_name = "luaBoundaryFunctionA"
    }
  },
  scattering_order = 1,
}

phys1 = lbs.DiscreteOrdinatesSolver.Create(lbs_block)
lbs.SetOptions(phys1, lbs_options)

--############################################### Initialize and Execute Solver
ss_solver = lbs.SteadyStateSolver.Create({lbs_solver_handle = phys1})

SolverInitialize(ss_solver)
SolverExecute(ss_solver)

--############################################### Get field functions
fflist,count = LBSGetScalarFieldFunctionList(phys1)

--############################################### Slice plot
slice2 = FFInterpolationCreate(SLICE)
FFInterpolationSetProperty(slice2,SLICE_POINT,0.0,0.0,0.025)
FFInterpolationSetProperty(slice2,ADD_FIELDFUNCTION,fflist[1])

FFInterpolationInitialize(slice2)
FFInterpolationExecute(slice2)

----############################################### Volume integrations
vol0 = mesh.RPPLogicalVolume.Create({infx=true, infy=true, infz=true})
ffi1 = FFInterpolationCreate(VOLUME)
curffi = ffi1
FFInterpolationSetProperty(curffi,OPERATION,OP_MAX)
FFInterpolationSetProperty(curffi,LOGICAL_VOLUME,vol0)
FFInterpolationSetProperty(curffi,ADD_FIELDFUNCTION,fflist[1])

FFInterpolationInitialize(curffi)
FFInterpolationExecute(curffi)
maxval = FFInterpolationGetValue(curffi)

Log(LOG_0,string.format("Max-value1=%.5f", maxval))

----############################################### Volume integrations
--ffi1 = FFInterpolationCreate(VOLUME)
--curffi = ffi1
--FFInterpolationSetProperty(curffi,OPERATION,OP_MAX)
--FFInterpolationSetProperty(curffi,LOGICAL_VOLUME,vol0)
--FFInterpolationSetProperty(curffi,ADD_FIELDFUNCTION,fflist[160])
--
--FFInterpolationInitialize(curffi)
--FFInterpolationExecute(curffi)
--maxval = FFInterpolationGetValue(curffi)
--
--Log(LOG_0,string.format("Max-value2=%.5e", maxval))

--############################################### Exports
if master_export == nil then
  FFInterpolationExportPython(slice2)
end

--############################################### Plots
if (location_id == 0 and master_export == nil) then
  local handle = io.popen("python ZPFFI00.py")
end
