-- 2D LinearBSolver test of a block of graphite with an air cavity. DSA and TG
-- SDM: PWLD
-- Test: WGS groups [0-62] Iteration    53 Residual 5.96018e-07 CONVERGED
-- and   WGS groups [63-167] Iteration    59 Residual 5.96296e-07 CONVERGED
num_procs = 4





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
N=20
L=100
--N=10
--L=200e6
xmin = -L/2
--xmin = 0.0
dx = L/N
for i=1,(N+1) do
  k=i-1
  mesh[i] = xmin + k*dx
end

chiMeshCreateUnpartitioned2DOrthoMesh(mesh,mesh)
--chiMeshCreateUnpartitioned1DOrthoMesh(mesh)
chiVolumeMesherExecute();

--############################################### Set Material IDs
chiVolumeMesherSetMatIDToAll(0)

vol1 = chiLogicalVolumeCreate(RPP,-10.0,10.0,-10.0,10.0,-1000,1000)
chiVolumeMesherSetProperty(MATID_FROMLOGICAL,vol1,1)

--############################################### Add materials
materials = {}
materials[1] = chiPhysicsAddMaterial("Test Material");
materials[2] = chiPhysicsAddMaterial("Test Material2");

chiPhysicsMaterialAddProperty(materials[1],TRANSPORT_XSECTIONS)
chiPhysicsMaterialAddProperty(materials[2],TRANSPORT_XSECTIONS)

chiPhysicsMaterialAddProperty(materials[1],ISOTROPIC_MG_SOURCE)
chiPhysicsMaterialAddProperty(materials[2],ISOTROPIC_MG_SOURCE)


--num_groups = 1
--chiPhysicsMaterialSetProperty(materials[1],TRANSPORT_XSECTIONS,
--        SIMPLEXS1,num_groups,1.0,0.999)
num_groups = 168
chiPhysicsMaterialSetProperty(materials[1],TRANSPORT_XSECTIONS,
  CHI_XSFILE,"tests/Transport_Steady/xs_graphite_pure.cxs")
chiPhysicsMaterialSetProperty(materials[2],TRANSPORT_XSECTIONS,
  CHI_XSFILE,"tests/Transport_Steady/xs_air50RH.cxs")

src={}
for g=1,num_groups do
  src[g] = 0.0
end
src[1] = 1.0
chiPhysicsMaterialSetProperty(materials[1],ISOTROPIC_MG_SOURCE,FROM_ARRAY,src)
src[1] = 0.0
chiPhysicsMaterialSetProperty(materials[2],ISOTROPIC_MG_SOURCE,FROM_ARRAY,src)

--############################################### Setup Physics
pquad0 = chiCreateProductQuadrature(GAUSS_LEGENDRE_CHEBYSHEV,2, 2,false)
chiOptimizeAngularQuadratureForPolarSymmetry(pqaud0, 4.0*math.pi)

lbs_block =
{
  num_groups = num_groups,
  groupsets =
  {
    {
      groups_from_to = {0, 62},
      angular_quadrature_handle = pquad0,
      angle_aggregation_num_subsets = 1,
      groupset_num_subsets = 1,
      inner_linear_method = "gmres",
      l_abs_tol = 1.0e-6,
      l_max_its = 1000,
      gmres_restart_interval = 30,
      apply_wgdsa = true,
      wgdsa_l_abs_tol = 1.0e-2,
    },
    {
      groups_from_to = {63, num_groups-1},
      angular_quadrature_handle = pquad0,
      angle_aggregation_num_subsets = 1,
      groupset_num_subsets = 1,
      inner_linear_method = "gmres",
      l_abs_tol = 1.0e-6,
      l_max_its = 1000,
      gmres_restart_interval = 30,
      apply_wgdsa = true,
      apply_tgdsa = true,
      wgdsa_l_abs_tol = 1.0e-2,
    },
  }
}

lbs_options =
{
  scattering_order = 1,
}

phys1 = lbs.DiscreteOrdinatesSolver.Create(lbs_block)
chiLBSSetOptions(phys1, lbs_options)

--############################################### Initialize and Execute Solver
chiSolverInitialize(phys1)

ss_solver = lbs.SteadyStateSolver.Create({lbs_solver_handle = phys1})

chiSolverExecute(ss_solver)

--############################################### Get field functions
fflist,count = chiLBSGetScalarFieldFunctionList(phys1)

--############################################### Exports
if (master_export == nil) then
  chiExportMultiFieldFunctionToVTK(fflist,"ZPhi")
end

--############################################### Plots
