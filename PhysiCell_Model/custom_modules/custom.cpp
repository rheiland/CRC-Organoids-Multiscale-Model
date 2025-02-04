/*
###############################################################################
# If you use PhysiCell in your project, please cite PhysiCell and the version #
# number, such as below:                                                      #
#                                                                             #
# We implemented and solved the model using PhysiCell (Version x.y.z) [1].    #
#                                                                             #
# [1] A Ghaffarizadeh, R Heiland, SH Friedman, SM Mumenthaler, and P Macklin, #
#     PhysiCell: an Open Source Physics-Based Cell Simulator for Multicellu-  #
#     lar Systems, PLoS Comput. Biol. 14(2): e1005991, 2018                   #
#     DOI: 10.1371/journal.pcbi.1005991                                       #
#                                                                             #
# See VERSION.txt or call get_PhysiCell_version() to get the current version  #
#     x.y.z. Call display_citations() to get detailed information on all cite-#
#     able software used in your PhysiCell application.                       #
#                                                                             #
# Because PhysiCell extensively uses BioFVM, we suggest you also cite BioFVM  #
#     as below:                                                               #
#                                                                             #
# We implemented and solved the model using PhysiCell (Version x.y.z) [1],    #
# with BioFVM [2] to solve the transport equations.                           #
#                                                                             #
# [1] A Ghaffarizadeh, R Heiland, SH Friedman, SM Mumenthaler, and P Macklin, #
#     PhysiCell: an Open Source Physics-Based Cell Simulator for Multicellu-  #
#     lar Systems, PLoS Comput. Biol. 14(2): e1005991, 2018                   #
#     DOI: 10.1371/journal.pcbi.1005991                                       #
#                                                                             #
# [2] A Ghaffarizadeh, SH Friedman, and P Macklin, BioFVM: an efficient para- #
#     llelized diffusive transport solver for 3-D biological simulations,     #
#     Bioinformatics 32(8): 1256-8, 2016. DOI: 10.1093/bioinformatics/btv730  #
#                                                                             #
###############################################################################
#                                                                             #
# BSD 3-Clause License (see https://opensource.org/licenses/BSD-3-Clause)     #
#                                                                             #
# Copyright (c) 2015-2021, Paul Macklin and the PhysiCell Project             #
# All rights reserved.                                                        #
#                                                                             #
# Redistribution and use in source and binary forms, with or without          #
# modification, are permitted provided that the following conditions are met: #
#                                                                             #
# 1. Redistributions of source code must retain the above copyright notice,   #
# this list of conditions and the following disclaimer.                       #
#                                                                             #
# 2. Redistributions in binary form must reproduce the above copyright        #
# notice, this list of conditions and the following disclaimer in the         #
# documentation and/or other materials provided with the distribution.        #
#                                                                             #
# 3. Neither the name of the copyright holder nor the names of its            #
# contributors may be used to endorse or promote products derived from this   #
# software without specific prior written permission.                         #
#                                                                             #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" #
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   #
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  #
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE   #
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         #
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        #
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    #
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     #
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     #
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  #
# POSSIBILITY OF SUCH DAMAGE.                                                 #
#                                                                             #
###############################################################################
*/

#include "../addons/keras/src/model.h"
#include "./custom.h"
#include "../modules/PhysiCell_settings.h"

// assume these files; can override in read_DNN()
auto WT_Model = keras2cpp::Model::load("Wild_Type.model");
auto KRAS_Model = keras2cpp::Model::load("KRAS.model");

void create_cell_types( void )
{
    read_DNN("Wild_Type.model", "KRAS.model");

	// set the random seed 
	SeedRandom( parameters.ints("random_seed") );  
	
	/* 
	   Put any modifications to default cell definition here if you 
	   want to have "inherited" by other cell types. 
	   
	   This is a good place to set default functions. 
	*/ 
	
	initialize_default_cell_definition(); 
	cell_defaults.phenotype.secretion.sync_to_microenvironment( &microenvironment ); 
	
	cell_defaults.functions.volume_update_function = standard_volume_update_function;
	cell_defaults.functions.update_velocity = standard_update_cell_velocity;

	cell_defaults.functions.update_migration_bias = NULL; 
	cell_defaults.functions.update_phenotype = NULL; // update_cell_and_death_parameters_O2_based; 
	cell_defaults.functions.custom_cell_rule = NULL; 
	cell_defaults.functions.contact_function = NULL; 
	
	cell_defaults.functions.add_cell_basement_membrane_interactions = NULL; 
	cell_defaults.functions.calculate_distance_to_membrane = NULL; 
	
	/*
	   This parses the cell definitions in the XML config file. 
	*/
	
	initialize_cell_definitions_from_pugixml(); 

	/*
	   This builds the map of cell definitions and summarizes the setup. 
	*/
		
	build_cell_definitions_maps(); 

	/*
	   This intializes cell signal and response dictionaries 
	*/

	setup_signal_behavior_dictionaries(); 	

	/* 
	   Put any modifications to individual cell definitions here. 
	   
	   This is a good place to set custom functions. 
	*/ 
	
	cell_defaults.functions.update_phenotype = phenotype_function; 
	cell_defaults.functions.custom_cell_rule = custom_function; 
	cell_defaults.functions.contact_function = contact_function; 
	
	/*
	   This builds the map of cell definitions and summarizes the setup. 
	*/
		
	display_cell_definitions( std::cout ); 
	
	return; 
}

void setup_microenvironment( void )
{
	// set domain parameters 
	
	// put any custom code to set non-homogeneous initial conditions or 
	// extra Dirichlet nodes here. 
	
	// initialize BioFVM 
	
	initialize_microenvironment(); 	
	
	return; 
}

void setup_tissue( void )
{
    // Load in DNN model (just 1 time)
    // auto WT_Model = keras2cpp::Model::load("Wild_Type.model");
    // WT_Model = keras2cpp::Model::load("Wild_Type.model");

	// place a cluster of tumor cells at the center 
 
	double tumor_radius = parameters.doubles( "tumor_radius" ); // 250.0; 
	
	// Parameter<double> temp; 
	
	int initial_tumor_radius = parameters.doubles( "tumor_radius" ); 
	
	Cell* pCell; 
	
    double cell_radius = cell_defaults.phenotype.geometry.radius; 
	double cell_spacing = 0.8 * 2.0 * cell_radius; 
	
	std::vector<std::vector<double>> positions = create_cell_circle_positions(cell_radius,initial_tumor_radius);

	std::cout << "Creating cells" << std::endl;
    
    for( int i=0; i < positions.size(); i++ )
    {
        pCell = create_cell(get_cell_definition("default"));
        pCell->functions.volume_update_function = NULL;
        pCell->assign_position( positions[i] );
	}
	
	// load cells from your CSV file (if enabled)
	load_cells_from_pugixml(); 		
	
	return; 
}


std::vector<std::vector<double>> create_cell_circle_positions(double cell_radius, double sphere_radius)
{
	std::vector<std::vector<double>> cells;
	int xc=0,yc=0,zc=0;
	double x_spacing= cell_radius*sqrt(3);
	double y_spacing= cell_radius*sqrt(3);

	std::vector<double> tempPoint(3,0.0);
	// std::vector<double> cylinder_center(3,0.0);
	
	for(double x=-sphere_radius;x<sphere_radius;x+=x_spacing, xc++)
	{
		for(double y=-sphere_radius;y<sphere_radius;y+=y_spacing, yc++)
		{
			tempPoint[1]=y + (xc%2) * cell_radius;
			tempPoint[0]=x;
			tempPoint[2]=0;
			if(sqrt(norm_squared(tempPoint))< sphere_radius)
			{ cells.push_back(tempPoint); }
		}
	}
	return cells;
}


std::vector<std::string> my_coloring_function( Cell* pCell )
{ return paint_by_number_cell_coloring(pCell); }

void phenotype_function( Cell* pCell, Phenotype& phenotype, double dt )
{ return; }

void custom_function( Cell* pCell, Phenotype& phenotype , double dt )
{  
    return;
} 

void contact_function( Cell* pMe, Phenotype& phenoMe , Cell* pOther, Phenotype& phenoOther , double dt )
{ return; } 

void read_DNN(std::string wt_filename, std::string kras_filename)
{
    WT_Model = keras2cpp::Model::load(wt_filename.c_str());
    KRAS_Model = keras2cpp::Model::load(kras_filename.c_str());
}

void simulate_DNN(double intracellular_dt )
{
    // keras2cpp::Tensor in{2};
    // keras2cpp::Tensor out;
    
    static int glc_index = microenvironment.find_density_index( "glucose" );
	static int gln_index = microenvironment.find_density_index( "glutamine" );
    static double exp_ave_n_cells = parameters.doubles("experimental_average_number_of_cells" );
    static double exp_vol_well = parameters.doubles("experimental_well_volume" );

    
    #pragma omp parallel for 
    for( int i=0; i < (*all_cells).size(); i++ )
    {
        // Wild type simulation
        if ((*all_cells)[i]->type == 0)
        {
            keras2cpp::Tensor in{2};
            keras2cpp::Tensor out;
            double glc_val_int = (*all_cells)[i]->nearest_density_vector()[glc_index];
            double gln_val_int = (*all_cells)[i]->nearest_density_vector()[gln_index];
            
            
            double u_glc = (*all_cells)[i]->custom_data[1] * exp_ave_n_cells / exp_vol_well * glc_val_int;
            double u_gln = (*all_cells)[i]->custom_data[2] * exp_ave_n_cells / exp_vol_well * gln_val_int;
            
            float fl_glc = u_glc;
            float fl_gln = u_gln;
            
            // std::cout << "Glucose = " << fl_glc << std::endl;
            //std::cout << "Glutamine = " << fl_gln << std::endl;    
            
            in.data_ = {fl_glc,fl_gln};
            // keras2cpp::Tensor out = WT_Model(in); // model evaluation
            out = WT_Model(in); // model evaluation
            //out.print();
            
            std::vector<double> result;
            result = out.result_vector();
            // std::vector<double> result = out.result_vector();
            
            double biomass_creation_flux = result[0];
            
            //(*all_cells)[i]->custom_data[biomass_vi]  = biomass_creation_flux;
            
            double volume_increase_ratio = 1 + ( biomass_creation_flux / 60 * intracellular_dt);
            (*all_cells)[i]->custom_data[0]  = biomass_creation_flux;
            (*all_cells)[i]->custom_data[3]  = fl_glc;
            (*all_cells)[i]->custom_data[4]  = fl_gln;
            (*all_cells)[i]->phenotype.volume.multiply_by_ratio(volume_increase_ratio);
            
            (*all_cells)[i]->phenotype.secretion.uptake_rates[glc_index]=fl_glc;
            (*all_cells)[i]->phenotype.secretion.uptake_rates[gln_index]=fl_gln;
            
            double cell_pressure = (*all_cells)[i]->state.simple_pressure;

            //rwh debug
            // if (PhysiCell_globals.current_time > 1139 && (i==0))
            // {
            //     std::cout << "time= " << PhysiCell_globals.current_time << ",  volume (cell 0)= " << (*all_cells)[0]->phenotype.volume.total << std::endl;
            // }
            if ( (*all_cells)[i]->phenotype.volume.total > 2494*2)
            {
                //if (cell_pressure < 0.8)
                //{
                    // std::cout << "Volume is big enough to divide" << std::endl;
                    // std::cout << "i= " << i << ",  volume= " << (*all_cells)[i]->phenotype.volume.total << std::endl;

                (*all_cells)[i]->phenotype.cycle.data.transition_rate(0,0) = 9e99;
                //(*all_cells)[i]->phenotype.volume.multiply_by_ratio(volume_increase_ratio/(*all_cells)[i]->phenotype.volume.total*2494*2);
                //}
                //else
                //{
                    //(*all_cells)[i]->phenotype.volume.multiply_by_ratio(1/volume_increase_ratio);
                //}
            }
            else
            {
                (*all_cells)[i]->phenotype.cycle.data.transition_rate(0,0) = 0.0;
            }
        }
        
        // KRAS type simulation
        else if ((*all_cells)[i]->type == 1)
        {
        }
    }
}

// original
// void simulate_DNN()
// {
// 	static int glc_index = microenvironment.find_density_index( "glucose" );
// 	static int gln_index = microenvironment.find_density_index( "glutamine" );
//     static double exp_ave_n_cells = parameters.doubles("experimental_average_number_of_cells" );
//     static double exp_vol_well = parameters.doubles("experimental_well_volume" );
	
// 	#pragma omp parallel for 
//     for( int i=0; i < (*all_cells).size(); i++ )
//     {
//         // Wild type simulation
//         if ((*all_cells)[i]->type == 0)
//         {           
//             keras2cpp::Tensor in{2}; //
//             in.data_ = {0.02,0.003};
//             //keras2cpp::Tensor out = WT_Model(in); // model evaluation
//             //out.print();
//         }
        
//         // KRAS type simulation
//         if ((*all_cells)[i]->type == 1)
//         {
            
            
//         }
// 	}
// 	return;
// }