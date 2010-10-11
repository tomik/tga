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

#ifndef P_MANAGER_
#define P_MANAGER_

/*! \file 
 * \brief Header file to p_manger.cc*/  

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include "log_manager.h"

using namespace std;


/**\brief Differs possible parametres according to type*/
enum Param_type {
	pt_string , //!< parametres has a string value ( for instance name of the file ) 
	pt_bool_pos, // !< it is "activating" ( positive ) switch - activates some treit
	pt_bool_neg, // !< it is "deactivating" ( negative ) switch - deactivates some implicit treit
	pt_int //!< parametres is an integer
};




/**\brief A record representing one parameter.
 * 
 * Param_record consists of three variables representing record in Params_manager::param_table.*/

struct Param_record {
	
	void * param_variable; //!< Pointer on corresponding variable.
	
	string param_token;//!< This is name of parametr: for instance "-st", "-sh" , ... 

	string param_token_alt; //!< This is alternative name : in GNU style e.g. "--transposition_tables", ...
	
	Param_type param_type; //!< Is is string, boolean or something else ? 
};

/**Maximum number of different parametres.*/
enum { MAX_PARAMS = 30 }; 

/**\brief Handling parametres operations.
 * 
 * Params_manager handles setting appropriate variables according to set parametres on command line.
		Besides array param_table which stores linking information between parametr and corresponding 
		variable, this class contains all variables set by parametres + functions to operate them -
		get_.. , set_...
*/
class Params_manager {  

private:
	
	Param_record param_table[MAX_PARAMS];  //!< Table with Param_record records, this is filled in the constructor and then it is used for matching parametres
	
	int param_table_size; //!< Size of the ::param_table.
	
	string fn_pos_in; 		//!< input position filename
//	string fn_res_out; 		//!< result out  == solved problem + variations

	string fn_sgf_list_in;  	//!< list of sgf pos. to compile - only compile_mode

	string fn_comp_db_in; 		//!< compiled database in
	string fn_comp_db_out; 	  //!< compiled database out - only compile_mode
	
	bool compile_mode; 		//!< Only compiling database of sgf positions
	bool pattern_match_mode; //!< Will he use pattern matcher or not 
	bool ab_pruning;  		//!< alafa beta pruning		
	bool multiple_pos_mode; //!< input file is list of positions
	bool sgf_testing_output;  //!< Long sgf output
//	bool fast_capture_recognition; // !< No eyes are controlled, only is checked if all white stones are not captured
	bool attacker_moves_first; //!< ,,Black to kill"
	bool defender_moves_first; //!< ,,White to live"
	bool ko_handling; //!< if switched off analyzer might return unaccurate desicion 
	bool pruned_sgf_output; //!< Only ,,interesting" variations are outputed  
	bool short_help;      //!< If switched short help pops up
	bool testing_area;   //!< switches program line in main.cc to testing area
	bool transposition_tables;   //!< uses transposition tables  
	bool save_tt;   //!< saves transposition after first search ( usually attacker's move )  
	bool iddfs; //!< activates iterative deepening depth first search;
	bool dyn_heur; //!< Using dynamic heuristics.
	bool stat_heur; //!< Using statical heuristic.
	int sgf_output_depth; //!< How deep is sgf output
	bool under_the_stones; //!< Is playing under the stones ( defender's ) allowed ?
	
public:
	/**\brief	Params_manager constructor.*/ 
	Params_manager();	

	bool get_under_the_stones() { return under_the_stones;}
	int get_sgf_output_depth() { return sgf_output_depth;}; 
	bool get_save_tt () { return save_tt; }
	bool get_transposition_tables () { return transposition_tables; }
	bool get_testing_area() { return testing_area; }
	bool get_short_help() { return short_help; }
	bool get_pruned_sgf_output () { return pruned_sgf_output;}
	bool get_ko_handling() { return ko_handling;}
	bool get_attacker_moves_first() { return attacker_moves_first;}
	bool get_defender_moves_first() { return defender_moves_first;}
	bool get_compile_mode() 	{ return compile_mode; }	
	bool get_pattern_match_mode() 	{ return pattern_match_mode; }	
	bool get_ab_prunning () { return ab_pruning ;}
	bool get_multiple_pos_mode () { return multiple_pos_mode ;}
	bool get_sgf_testing_output () { return sgf_testing_output ;}
	bool get_iddfs () { return iddfs ;}
	bool get_stat_heur() { return stat_heur; }
	bool get_dyn_heur() { return dyn_heur; }
//	bool get_fast_capture_recognition () { return fast_capture_recognition;}
	
	string get_fn_pos_in () 	{ return fn_pos_in; }
////	string get_fn_res_out()	  { return fn_res_out;}
	string get_fn_sgf_list_in () { return fn_sgf_list_in; }
	string get_fn_comp_db_in  () { return fn_comp_db_in; }
	string get_fn_comp_db_out () { return fn_comp_db_out; }
	
	/**\brief Setting variables.*/
	bool set_variable(string token, string value); /*[F] returns false if token or value are not correct -> print_short_help() is called and program ends*/

	/**\brief Prints help*/
	void print_short_help();   /*[F] prints information about usage of program */

	void set_attacker_moves_first ( bool value_ ) { attacker_moves_first = value_; }
	void set_defender_moves_first ( bool value_ ) { defender_moves_first = value_; }
	void set_sgf_output_depth(int new_value_) { sgf_output_depth=new_value_;}; 

};

extern Params_manager params_manager; //instance

#endif
