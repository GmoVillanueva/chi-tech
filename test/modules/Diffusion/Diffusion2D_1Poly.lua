-- 2D Diffusion test with Dirichlet BCs.
-- SDM: PWLC
-- Test: Max-value=0.29480
num_procs = 1





--############################################### Check num_procs
if (check_num_procs==nil and chi_number_of_processes ~= num_procs) then
    chiLog(LOG_0ERROR,"Incorrect amount of processors. " ..
                      "Expected "..tostring(num_procs)..
                      ". Pass check_num_procs=false to override if possible.")
    os.exit(false)
end

--############################################### Setup mesh
chiMeshHandlerCreate()

mesh={}
N=32
L=2.0
xmin = -1.0
dx = L/N
for i=1,(N+1) do
    k=i-1
    mesh[i] = xmin + k*dx
end
chiMeshCreateUnpartitioned2DOrthoMesh(mesh,mesh)
chiVolumeMesherExecute();

--############################################### Set Material IDs
vol0 = chi_mesh.RPPLogicalVolume.Create({infx=true, infy=true, infz=true})
chiVolumeMesherSetProperty(MATID_FROMLOGICAL,vol0,0)

--############################################### Add materials
materials = {}
materials[0] = chiPhysicsAddMaterial("Test Material");

chiPhysicsMaterialAddProperty(materials[0],SCALAR_VALUE)
chiPhysicsMaterialSetProperty(materials[0],SCALAR_VALUE,SINGLE_VALUE,1.0)

--############################################### Setup Physics
phys1 = chiDiffusionCreateSolver()
chiSolverSetBasicOption(phys1,"discretization_method","PWLC")
chiSolverSetBasicOption(phys1,"residual_tolerance",1.0e-4)

--############################################### Set boundary conditions
--chiDiffusionSetProperty(phys1,"boundary_type",0,"reflecting",1.0)
--chiDiffusionSetProperty(phys1,"boundary_type",1,"vacuum",2.0)
--chiDiffusionSetProperty(phys1,"boundary_type",2,"reflecting",3.0)
--chiDiffusionSetProperty(phys1,"boundary_type",3,"vacuum",4.0)

--############################################### Initialize and Execute Solver
chiDiffusionInitialize(phys1)
chiDiffusionExecute(phys1)

--############################################### Get field functions
fftemp,count = chiSolverGetFieldFunctionList(phys1)

--############################################### Slice plot
slice2 = chiFFInterpolationCreate(SLICE)
chiFFInterpolationSetProperty(slice2,SLICE_POINT,0.0,0.0,0.025)
chiFFInterpolationSetProperty(slice2,ADD_FIELDFUNCTION,fftemp[1])

chiFFInterpolationInitialize(slice2)
chiFFInterpolationExecute(slice2)

--############################################### Line plot
line0 = chiFFInterpolationCreate(LINE)
chiFFInterpolationSetProperty(line0,LINE_FIRSTPOINT,-1.0,0.1,0.0)
chiFFInterpolationSetProperty(line0,LINE_SECONDPOINT, 1.0,0.1,0.0)
chiFFInterpolationSetProperty(line0,LINE_NUMBEROFPOINTS, 100)
chiFFInterpolationSetProperty(line0,ADD_FIELDFUNCTION,fftemp[1])

chiFFInterpolationInitialize(line0)
chiFFInterpolationExecute(line0)

--############################################### Volume integrations
ffi1 = chiFFInterpolationCreate(VOLUME)
curffi = ffi1
chiFFInterpolationSetProperty(curffi,OPERATION,OP_MAX)
chiFFInterpolationSetProperty(curffi,LOGICAL_VOLUME,vol0)
chiFFInterpolationSetProperty(curffi,ADD_FIELDFUNCTION,fftemp[1])

chiFFInterpolationInitialize(curffi)
chiFFInterpolationExecute(curffi)
maxval = chiFFInterpolationGetValue(curffi)

chiLog(LOG_0,string.format("Max-value=%.5f", maxval))

--############################################### Exports
if (master_export == nil) then
    chiFFInterpolationExportPython(slice2)
    chiFFInterpolationExportPython(line0)
    chiExportFieldFunctionToVTK(fftemp,"ZPhi")
end

--############################################### Plots
if (chi_location_id == 0 and master_export == nil) then
    local handle = io.popen("python3 ZPFFI00.py")
    local handle = io.popen("python3 ZLFFI10.py")
end
