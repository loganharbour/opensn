--############################################### Setup mesh
nodes={}
N=32
ds=2.0/N
for i=0,N do
    nodes[i+1] = -1.0 + i*ds
end
meshgen1 = mesh.OrthogonalMeshGenerator.Create({ node_sets = {nodes,nodes,nodes} })
mesh.MeshGenerator.Execute(meshgen1)

-- Set Material IDs
vol0 = logvol.Create(RPP,-1000,1000,-1000,1000,-1000,1000)
mesh.SetMaterialIDFromLogicalVolume(vol0,0)

--############################################### Add material
material0 = mat.AddMaterial("Test Material");

mat.AddProperty(material0, TRANSPORT_XSECTIONS)
num_groups = 1
Sigma_t = 1.0
scattering_ratio = 0.2
mat.SetProperty(material0, TRANSPORT_XSECTIONS, SIMPLEXS1, num_groups, Sigma_t, scattering_ratio)

--############################################### Setup Physics
phys1 = LBSCreateSolver()
SolverAddRegion(phys1,region1)

for k=1,num_groups do
    LBSCreateGroup(phys1)
end

pquad = aquad.CreateProductQuadrature(GAUSS_LEGENDRE_CHEBYSHEV, 4, 4)

--========== Groupset def
gs0 = LBSCreateGroupset(phys1)
LBSGroupsetAddGroups(phys1,gs0,0,num_groups-1)
LBSGroupsetSetQuadrature(phys1,gs0,pquad)
LBSGroupsetSetAngleAggregationType(phys1,gs0,LBSGroupset.ANGLE_AGG_SINGLE)
LBSGroupsetSetIterativeMethod(phys1,gs0,NPT_GMRES_CYCLES)

--========== Boundary conditions
bsrc = {}
for k=1,num_groups do
    bsrc[k] = 0.0
end
bsrc[1] = 0.5
LBSSetProperty(phys1,BOUNDARY_CONDITION,
                  YMIN,LBSBoundaryTypes.INCIDENT_ISOTROPIC,bsrc);

--========== Solvers
LBSSetProperty(phys1,DISCRETIZATION_METHOD,PWLD)
LBSSetProperty(phys1,SCATTERING_ORDER,0)

LBSInitialize(phys1)
LBSExecute(phys1)

--############################################### Setup Output
fflist,count = lbs.GetScalarFieldFunctionList(phys1)

cline = fieldfunc.FFInterpolationCreate(LINE)
fieldfunc.SetProperty(cline,LINE_FIRSTPOINT,0.0,-1.0,-1.0)
fieldfunc.SetProperty(cline,LINE_SECONDPOINT,0.0, 1.0,1.0)
fieldfunc.SetProperty(cline,LINE_NUMBEROFPOINTS, 50)

fieldfunc.SetProperty(cline,ADD_FIELDFUNCTION,fflist[1])


fieldfunc.Initialize(cline)
fieldfunc.Execute(cline)
fieldfunc.ExportPython(cline)

fieldfunc.ExportToVTK(fflist[1],"Tutorial3Output","Phi")

if (location_id == 0) then
    local handle = io.popen("python ZLFFI00.py")
end
