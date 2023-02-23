#include "fieldfunction.h"

#include "chi_runtime.h"
#include "chi_log.h"

#include "ChiMesh/MeshContinuum/chi_meshcontinuum.h"
#include "ChiMesh/MeshContinuum/chi_meshcontinuum_vtk_utils.h"

#include "ChiMath/SpatialDiscretization/spatial_discretization.h"

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>

//###################################################################
/**Export multiple field functions to VTK.*/
void chi_physics::FieldFunction::
  ExportMultipleToVTK(
    const std::string &file_base_name,
    const std::vector<std::shared_ptr<const FieldFunction>> &ff_list)
{
  const std::string fname = "chi_physics::FieldFunction::ExportMultipleToVTK";
  chi::log.Log() << "Exporting field functions to VTK with file base \""
                 << file_base_name << "\"";

  if (ff_list.empty())
    throw std::logic_error(fname + ": Cannot be used with empty field-function"
                                   " list");

  //============================================= Setup master and check slaves
  const auto& master_ff_ptr = ff_list.front();
  const auto& master_ff = *master_ff_ptr;

  for (const auto& ff_ptr : ff_list)
    if (ff_ptr != master_ff_ptr)
      if (ff_ptr->m_sdm->ref_grid != master_ff_ptr->m_sdm->ref_grid)
        throw std::logic_error(fname +
        ": Cannot be used with field functions based on different grids.");

  //============================================= Get grid
  const auto& grid = *master_ff.m_sdm->ref_grid;

  auto ugrid = chi_mesh::PrepareVtkUnstructuredGrid(grid);

  //============================================= Upload cell/point data
  auto cell_data = ugrid->GetCellData();
  auto point_data = ugrid->GetPointData();
  for (const auto& ff_ptr : ff_list)
  {
    const auto field_vector = ff_ptr->GetGhostedFieldVector();

    const auto& uk_man = ff_ptr->m_unknown_manager;
    const auto& unknown = ff_ptr->m_unknown;
    const auto& sdm = ff_ptr->m_sdm;

    for (uint c=0; c<unknown.num_components; ++c)
    {
      const std::string component_name = ff_ptr->m_text_name +
                                         unknown.text_name +
                                         unknown.component_text_names[c];
      vtkNew<vtkDoubleArray> point_array;
      vtkNew<vtkDoubleArray> cell_array;

      point_array->SetName(component_name.c_str());
      cell_array->SetName(component_name.c_str());

      //Populate the array here
      for (const auto& cell : grid.local_cells)
      {
        const size_t num_nodes = sdm->GetCellNumNodes(cell);

        if (num_nodes == cell.vertex_ids.size())
        {
          double node_average = 0.0;
          for (int n=0; n<num_nodes; ++n)
          {
            const int64_t nmap = sdm->MapDOFLocal(cell,n,uk_man,0,c);

            const double field_value = field_vector[nmap];

            point_array->InsertNextValue(field_value);
            node_average += field_value;
          }//for node
          node_average /= static_cast<double>(num_nodes);
          cell_array->InsertNextValue(node_average);
        }
        else
        {
          double node_average = 0.0;
          for (int n=0; n<num_nodes; ++n)
          {
            const int64_t nmap = sdm->MapDOFLocal(cell,n,uk_man,0,c);

            const double field_value = field_vector[nmap];
            node_average += field_value;
          }//for node
          node_average /= static_cast<double>(num_nodes);
          cell_array->InsertNextValue(node_average);
          for (int n=0; n<cell.vertex_ids.size(); ++n)
          {
            point_array->InsertNextValue(node_average);
          }//for vertex
        }

      }//for cell

      point_data->AddArray(point_array);
      cell_data->AddArray(cell_array);
    }//for component
  }//for ff_ptr

  chi_mesh::WritePVTUFiles(ugrid, file_base_name);

  chi::log.Log() << "Done exporting field functions to VTK.";
}