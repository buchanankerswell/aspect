/*
  Copyright (C) 2015 - 2023 by the authors of the ASPECT code.

  This file is part of ASPECT.

  ASPECT is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  ASPECT is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ASPECT; see the file LICENSE.  If not see
  <http://www.gnu.org/licenses/>.
*/


#include <aspect/postprocess/visualization/artificial_viscosity_composition.h>


namespace aspect
{
  namespace Postprocess
  {
    namespace VisualizationPostprocessors
    {
      template <int dim>
      ArtificialViscosityComposition<dim>::
      ArtificialViscosityComposition ()
        :
        CellDataVectorCreator<dim>("m/s/s")
      {}

      template <int dim>
      std::pair<std::string, std::unique_ptr<Vector<float>>>
      ArtificialViscosityComposition<dim>::execute() const
      {
        Assert(this->n_compositional_fields()>0,
               ExcMessage ("The artificial viscosity for compositional fields can "
                           "only be calculated if compositional fields are used in the simulation."));

        std::pair<std::string, std::unique_ptr<Vector<float>>>
        return_value ("artificial_viscosity_composition",
                      std::make_unique<Vector<float>>(this->get_triangulation().n_active_cells()));
        this->get_artificial_viscosity_composition(*return_value.second, compositional_field);

        // The function we call above sets the artificial viscosity to
        // signaling_nan on all artificial cells and, possibly, ghost cells.
        // This runs into trouble in DataOut that wants to copy this vector
        // from Vector<float> to Vector<double>, and the conversion trips
        // up over the NaNs, causing a floating point exception.
        //
        // To avoid this, strip out the NaNs and instead set these values
        // to zero -- we won't be outputting these values anyway.
        for (const auto &cell : this->get_triangulation().active_cell_iterators())
          if (cell->is_locally_owned() == false)
            (*return_value.second)[cell->active_cell_index()] = 0;

        return return_value;
      }

      template <int dim>
      void
      ArtificialViscosityComposition<dim>::
      declare_parameters (ParameterHandler &prm)
      {
        prm.enter_subsection("Postprocess");
        {
          prm.enter_subsection("Visualization");
          {
            prm.enter_subsection("Artificial viscosity composition");
            {
              prm.declare_entry ("Name of compositional field", "",
                                 Patterns::Anything(),
                                 "The name of the compositional field whose output "
                                 "should be visualized. ");
            }
            prm.leave_subsection();
          }
          prm.leave_subsection();
        }
        prm.leave_subsection();
      }

      template <int dim>
      void
      ArtificialViscosityComposition<dim>::parse_parameters (ParameterHandler &prm)
      {
        prm.enter_subsection("Postprocess");
        {
          prm.enter_subsection("Visualization");
          {
            prm.enter_subsection("Artificial viscosity composition");
            {
              const std::string field_name = prm.get("Name of compositional field");

              AssertThrow(this->introspection().compositional_name_exists(field_name),
                          ExcMessage("No compositional field with name <" +
                                     field_name +
                                     "> exists for which you want to visualize the artificial viscosity."));

              compositional_field = this->introspection().compositional_index_for_name(field_name);
            }
            prm.leave_subsection();
          }
          prm.leave_subsection();
        }
        prm.leave_subsection();
      }
    }
  }
}


// explicit instantiations
namespace aspect
{
  namespace Postprocess
  {
    namespace VisualizationPostprocessors
    {
      ASPECT_REGISTER_VISUALIZATION_POSTPROCESSOR(ArtificialViscosityComposition,
                                                  "artificial viscosity composition",
                                                  "A visualization output object that generates output "
                                                  "showing the value of the artificial viscosity for a "
                                                  "compositional field on each cell."
                                                  "\n\n"
                                                  "Physical units: $\\frac{\\text{m}^2}{\\text{s}}$.")
    }
  }
}
