/*
	TGA ( Tomik's Go Analyzator ), life & death problem solver in the game of Go
	Copyright (C) 2006 Tomas Kozelek 

	TGA is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	TGA is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


/**\file p_manager.cc
 * \brief Contains implementation of methods handling resolving paramatres ( \ref page_1 ).
*/
#include "p_manager.h"

/**
 * Table of corresponding parametres and variables is filled in this constructor.
*/
Params_manager::Params_manager() { 
	sgf_output_depth = 0; //this means unlimited
	under_the_stones = true; // under the stones are implicitly on
	iddfs = false;
	save_tt = true;
	transposition_tables = true;	
	testing_area = false;
	short_help = false;
	ko_handling = true;
	defender_moves_first = false;
	attacker_moves_first = false;
	pattern_match_mode = false;
	compile_mode = false;	
	ab_pruning = true;
	multiple_pos_mode = false;
	dyn_heur = true;
	stat_heur = true;
//	fast_capture_recognition = false;
	sgf_testing_output = false;

	/*[C] input position*/
	param_table[0].param_token = "-i";
	param_table[0].param_token_alt = "--input";
	param_table[0].param_variable = ( string * ) ( & fn_pos_in ) ;
	param_table[0].param_type = pt_string;
	
	/*[C] output file
	param_table[1].param_token = "-o";
	param_table[1].param_variable = ( string * ) ( & fn_res_out );
	param_table[1].param_type = pt_string;*/
	
	/*[C] sgf_list
	param_table[1].param_token = "-s";
	param_table[1].param_variable = ( string * ) ( & fn_sgf_list_in );
	param_table[1].param_type = pt_string;*/
	
	/*[C] database compilation mode
	param_table[2].param_token = "-c";
	param_table[2].param_variable = ( bool * ) ( & compile_mode );
	param_table[2].param_type = pt_bool;*/
	
	/*[C] compiled database
	param_table[3].param_token = "-d";
	param_table[3].param_variable = ( string * ) ( & fn_comp_db_in);
	param_table[3].param_type = pt_string;*/
	
	/*[C] destination for compiled database
	param_table[4].param_token = "-do";
	param_table[4].param_variable = ( string * ) ( & fn_comp_db_out);
	param_table[4].param_type = pt_string;*/
	
	/*[C] pattern matcher mode
	param_table[5].param_token = "-p";
	param_table[5].param_variable = ( bool * ) ( & pattern_match_mode);
	param_table[5].param_type = pt_bool;*/
	
	/*[C] alfa beta pruning*/
	param_table[1].param_token = "-nab";
	param_table[1].param_token_alt = "--no_alpha_beta";
	param_table[1].param_variable = ( bool * ) ( & ab_pruning);
	param_table[1].param_type = pt_bool_neg;

	/*[C] Multiple positions mode*/
	param_table[2].param_token = "-mp";
	param_table[2].param_token_alt = "--multiple_pos_mode";
	param_table[2].param_variable = ( bool * ) ( & multiple_pos_mode);
	param_table[2].param_type = pt_bool_pos;

	/*[C] Sgf_testing_output ( much longer ) */
	param_table[3].param_token = "-to";
	param_table[3].param_token_alt = "--sgf_testing_output";
	param_table[3].param_variable = ( bool * ) ( & sgf_testing_output);
	param_table[3].param_type = pt_bool_pos;
	
	/*[C] fast_life_recognition
	param_table[10].param_token = "-fc";
	param_table[10].param_variable = ( bool * ) ( & fast_capture_recognition);
	param_table[10].param_type = pt_bool;*/

	/*[C] attacker_moves_first*/
	param_table[4].param_token = "-af";
	param_table[4].param_token_alt = "--attacker_first";
	param_table[4].param_variable = ( bool * ) ( & attacker_moves_first );
	param_table[4].param_type = pt_bool_pos;

	
	/*[C] defender_moves_first*/
	param_table[5].param_token = "-df";
	param_table[5].param_token_alt = "--defender_first";
	param_table[5].param_variable = ( bool * ) ( & defender_moves_first );
	param_table[5].param_type = pt_bool_pos;
		
	/*[C] ko handling */
	param_table[6].param_token = "-nkh";
	param_table[6].param_token_alt = "--no_ko_handling";
	param_table[6].param_variable = ( bool * ) ( & ko_handling );
	param_table[6].param_type = pt_bool_neg;
	
	/*[C] pruned sgf output ( only ,,interesting variations )*/
	param_table[7].param_token = "-ps";
	param_table[7].param_token_alt = "--prune_sgf_output";
	param_table[7].param_variable = ( bool * ) ( & pruned_sgf_output );
	param_table[7].param_type = pt_bool_pos;
	
	/*[C] short_help*/
	param_table[8].param_token = "-h";
	param_table[8].param_token_alt = "--help";
	param_table[8].param_variable = ( bool * ) ( & short_help );
	param_table[8].param_type = pt_bool_pos;

	/*[C] testing_area*/
	param_table[9].param_token = "-ta";
	param_table[9].param_token_alt = "--testing_area";
	param_table[9].param_variable = ( bool * ) ( & testing_area );
	param_table[9].param_type = pt_bool_pos;

	/*[C] use transposition tables*/
	param_table[10].param_token = "-ntt";
	param_table[10].param_token_alt = "--no_t_tables";
	param_table[10].param_variable = ( bool * ) ( & transposition_tables );
	param_table[10].param_type = pt_bool_neg;

	/*[C] save_transposition tables after first search*/
	param_table[11].param_token = "-nst";
	param_table[11].param_token_alt = "--no_save_t_tables";
	param_table[11].param_variable = ( bool * ) ( & save_tt);
	param_table[11].param_type = pt_bool_neg;

  /*[C] activate iterative deepening depth first search*/
	param_table[12].param_token = "-id";
	param_table[12].param_token_alt = "--iterative_deepening";
	param_table[12].param_variable = ( bool * ) ( & iddfs);
	param_table[12].param_type = pt_bool_pos;
	
  /*[C] Prevents using dynamic heuristics*/
	param_table[13].param_token = "-ndh";
	param_table[13].param_token_alt = "--no_dynamic_heuristics";
	param_table[13].param_variable = ( bool * ) ( & dyn_heur);
	param_table[13].param_type = pt_bool_neg;
	
  /*[C] Prevents using statical heuristics*/
	param_table[14].param_token = "-nsh";
	param_table[14].param_token_alt = "--no_static_heuristics";
	param_table[14].param_variable = ( bool * ) ( & stat_heur);
	param_table[14].param_type = pt_bool_neg;
	
  /*[C] Under the stones are not taken into account */
	param_table[15].param_token = "-nus";
	param_table[15].param_token_alt = "--no_under_the_stones";
	param_table[15].param_variable = ( bool * ) ( & under_the_stones);
	param_table[15].param_type = pt_bool_neg;

  /*[C] Depht of sgf output */
	param_table[16].param_token = "-od";
	param_table[16].param_token_alt = "--sgf_output_depth";
	param_table[16].param_variable = ( int * ) ( & sgf_output_depth);
	param_table[16].param_type = pt_int;


	param_table_size = 17;
	
	};

	
/**
 * \param[in] token String token which represents a parametr ( e.g. -i , -tt , ... )
 * \param[in] value String value of given parameter ( since most of the tokens are connected with boolean values \c true / \c false this field is empty )
		Method looks up in Params_manager::param_table given token and sets corresponding variable ( from the record in table ) on given value ( or on \c true
		if token is of boolean type ).
*/	
bool Params_manager::set_variable(string token,  string value) {
	if ( token == "" ) 
		return true;
	bool consistent = false;
		for ( int i = 0 ; i < param_table_size; i++ ) 
			if ( param_table[i].param_token == 	token || param_table[i].param_token_alt == token) {
				consistent = true;
				switch ( param_table[i].param_type ) {
					case pt_string :
						* ( (string *) (param_table[i].param_variable) ) = value; break;
					case pt_bool_pos :
						* ( (bool *) (param_table[i].param_variable) ) =  true ; break;
					case pt_bool_neg :
						* ( (bool *) (param_table[i].param_variable) ) =  false ; break;
					case pt_int :
						* ( (int *) (param_table[i].param_variable) ) = atoi( value.c_str() ); break;
						//default is false
					default : break;
				}
			} // if 
		if ( consistent ) 
			return true;
		else { 
			/*LOGGING*/ log_m.buff << "Unknown option " << token << ", program terminated." << endl; log_m.flush_buffer();
			print_short_help();
			return false;
		}
}

void Params_manager::print_short_help() {
	
	cout << endl << "Usage:  tga_ts [options] [-od number] -i input_file " << endl <<
		endl <<
		"Options:" << endl <<
		"  -nab  ( --no_alpha_beta )         deactivate alpha beta prunning" << endl << 
	 	"  -af  ( --attacker_first )         searches only for attacker to move "  << endl <<
 	  "  -df  ( --defender_first )         searches only for defender to move " << endl <<
	  "  -to  ( --sgf_testing_output )     sgf testing output " << endl <<
		"  -nkh ( --no_ko_handling )         no ko handling - ! this might cause some problems to be solved incorectly ( testing )" << endl <<
		"  -ta  ( --testing_area )           main program line is switched to testing area ( testing )" << endl <<
		"  -ps  ( --pruned_sgf )             pruning sgf output " << endl <<
		"  -mp  ( --multiple_pos_mode )      multiple position mode ( after -i expects list of position to search ) " << endl <<
		"  -ntt  ( --no_t_tables )           deactivate transposition tables" << endl <<
		"  -nst  ( --no_save_t_tables )      deactivate saving transposition tables after first ( attacker's ) search ( slows down second == defender's search ) "	<< endl <<
		"  -id  ( --iterative_deepening )    activates iterative deepening depth first search"
		<< endl <<
		"  -ndh ( --no_dynamic_heuristics )  prevents using dynamic heuristics " << endl <<
		"  -nsh ( --no_static_heuristics )   prevents using static heuristics " << endl << 
		"  -nus ( --no_under_the_stones )    prevents potential under the stones analyzis - ! this might cause some problems to be solved incorectly ( testing )" << endl << 
		"  -od  ( --sgf_output_depth )       following number sets sgf output depth" << endl << 
		"  -h   ( --help )                   this help ( it shows up when no option | argument is given as well ) " << endl <<
			endl << 
		
"This is a program by Tomas Kozelek (tomik84@gmail.com). " << endl << 
"For more information see documentation in section ./doc." << endl;
}


	

/**/
