#include "D_DO_Transient/lbts_transient_solver.h"

/**Computes the delayed neutron factor.*/
double lbs::DiscOrdTransientSolver::ComputeBeta()
{
  if (num_precursors_ == 0 || !options_.use_precursors)
    return 0.0;

  //compute total fission neutron production
  const double FPR = ComputeFissionProduction(phi_new_local_);

  //compute delayed fission neutron production
  double localDNPR = 0.0;
  for (const auto& cell : grid_ptr_->local_cells)
  {
    const auto& transport_view = cell_transport_views_[cell.local_id_];
    const double cell_volume = transport_view.Volume();
    const size_t dof_map = cell.local_id_ + max_precursors_per_material_;

    auto xs = transport_view.XS();

    //skip cell if not fissionable
    if (!xs.is_fissionable_)
      continue;

    //============================= Loop over groupsets
    for (const auto& groupset : groupsets_)
    {
      auto gs_i = static_cast<size_t>(groupset.groups_.front().id_);
      auto gs_f = static_cast<size_t>(groupset.groups_.back().id_);

      //============================= Loop over groupset groups
      for (size_t g = gs_i; g <= gs_f; ++g)
        for (unsigned int j = 0; j < xs.num_precursors_; ++j)
        {
          const auto& precursor = xs.precursors_[j];

          //TODO: Verify that this is correct.
          localDNPR += precursor.emission_spectrum[g] *
                       precursor.decay_constant *
                       precursor_new_local_[dof_map + j] *
                       cell_volume;
        }
    }
  }

  //============================================= Allreduce global DNPR
  double DNPR = 0.0;
  MPI_Allreduce(&localDNPR, &DNPR, 1,
                MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  return DNPR / (DNPR + FPR);
}