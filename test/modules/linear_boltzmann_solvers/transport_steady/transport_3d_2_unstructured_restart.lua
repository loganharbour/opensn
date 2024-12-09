-- 3D transport restart test with vacuum and incident-isotropic boundary condtions
-- SDM: PWLD
-- Test: Max-value=5.88996

-- Set and check number of processors
num_procs = 4
if check_num_procs == nil and number_of_processes ~= num_procs then
  log.Log(
    LOG_0ERROR,
    "Incorrect amount of processors. "
      .. "Expected "
      .. tostring(num_procs)
      .. ". Pass check_num_procs=false to override if possible."
  )
  os.exit(false)
end

-- Setup mesh
meshgen1 = mesh.ExtruderMeshGenerator.Create({
  inputs = {
    mesh.FromFileMeshGenerator.Create({
      filename = "../../../assets/mesh/TriangleMesh2x2Cuts.obj",
    }),
  },
  layers = { { z = 0.4, n = 2 }, { z = 0.8, n = 2 }, { z = 1.2, n = 2 }, { z = 1.6, n = 2 } }, -- layers
  partitioner = mesh.KBAGraphPartitioner.Create({
    nx = 2,
    ny = 2,
    xcuts = { 0.0 },
    ycuts = { 0.0 },
  }),
})
grid = meshgen1:Execute()

-- Set material IDs
vol0 = logvol.RPPLogicalVolume.Create({ infx = true, infy = true, infz = true })
grid:SetMaterialIDFromLogicalVolume(vol0, 0, true)

vol1 =
  logvol.RPPLogicalVolume.Create({ xmin = -0.5, xmax = 0.5, ymin = -0.5, ymax = 0.5, infz = true })
grid:SetMaterialIDFromLogicalVolume(vol1, 1, true)

-- Add materials
materials = {}
materials[1] = mat.AddMaterial("Test Material")
materials[2] = mat.AddMaterial("Test Material2")

num_groups = 1
xs_1g = xs.CreateSimpleOneGroup(1000.0, 0.9999)
materials[1]:SetTransportXSections(xs_1g)
materials[2]:SetTransportXSections(xs_1g)

src = {}
for g = 1, num_groups do
  src[g] = 0.5
end

mg_src = xs.IsotropicMultiGroupSource.FromArray(src)
materials[1]:SetIsotropicMGSource(mg_src)
materials[2]:SetIsotropicMGSource(mg_src)

--Setup physics
pquad0 = aquad.CreateProductQuadrature(GAUSS_LEGENDRE_CHEBYSHEV, 2, 2)

lbs_block = {
  mesh = grid,
  num_groups = num_groups,
  groupsets = {
    {
      groups_from_to = { 0, 0 },
      angular_quadrature = pquad0,
      angle_aggregation_num_subsets = 1,
      groupset_num_subsets = 1,
      inner_linear_method = "petsc_gmres",
      l_abs_tol = 1.0e-6,
      l_max_its = 10000,
      gmres_restart_interval = 100,
    },
  },
}
bsrc = {}
for g = 1, num_groups do
  bsrc[g] = 0.0
end
bsrc[1] = 1.0 / 4.0 / math.pi
lbs_options = {
  boundary_conditions = {
    { name = "zmax", type = "isotropic", group_strength = bsrc },
  },
  scattering_order = 1,
  save_angular_flux = true,
  --write_restart_time_interval = 60,
  --write_restart_path = "transport_3d_2_unstructured_restart/transport_3d_2_unstructured",
  read_restart_path = "transport_3d_2_unstructured_restart/transport_3d_2_unstructured",
}

phys1 = lbs.DiscreteOrdinatesSolver.Create(lbs_block)
phys1:SetOptions(lbs_options)

--Initialize and execute solver
ss_solver = lbs.SteadyStateSolver.Create({ lbs_solver = phys1 })

ss_solver:Initialize()
ss_solver:Execute()

--Get field functions
fflist = lbs.GetScalarFieldFunctionList(phys1)

-- Volume integrations
ffi1 = fieldfunc.FieldFunctionInterpolationVolume.Create()
curffi = ffi1
curffi:SetOperationType(OP_MAX)
curffi:SetLogicalVolume(vol0)
curffi:AddFieldFunction(fflist[1])

curffi:Initialize()
curffi:Execute()
maxval = curffi:GetValue()

log.Log(LOG_0, string.format("Max-value1=%.5e", maxval))

ffi1 = fieldfunc.FieldFunctionInterpolationVolume.Create()
