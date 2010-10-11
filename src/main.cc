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
	aong with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
 [V] == variable
 [E] == enum
 [F] == function
 [T] == testing
 [TU]== tuning
 [TD]== todo
 */

/** \file main.cc
 * \brief File related to search starting ( \ref page_2 )
 *
 * In this module, main() function is implemented. It handles parsing program arguments and according to them coordinates other modules
 */
#include "main.h"

using namespace std;

/**\brief Analyzes single position stored in file with filename as first argument, when second argument stone color moves first */
Group_status analyze_single_position ( string , Stone_color, double * , string ); 
/**\brief Completely solves given problem*/
void handle_single_problem ( string ,  bool  , int * , double * );
/**\brief Prints output.*/
void print_group_status ( Stone_color , Group_status  ) ;
/**\brief Generates output filename from input string*/
string get_out_fn_base ( string );

string g_filename; // TODO TESTING REMOVE
bool g_print_info = true; //!< global variable to recognize whether info on the problem was already printed

/**
 * Function main at first parses input arguments and according to them decides further actions.*/

int main(int argc , char * argv[]) {

/*LOGGING	cout << "Hello world, this is tga_ts.out" <<endl; /**/

string act_token; /*[V] Actual token - identifiers ( like -i for input )*/
string act_value; /*[V] Value of act_token*/

srand( (unsigned long ) 87341317495209345ULL );
		
// srand ((unsigned)time(  NULL )); /*[C] init random number seed, very important */


/*At first it is neccessary to manager parametres*/

	if ( argc == 1 ) { /*[C] No parametres => prints short help*/
		 params_manager.print_short_help();
		 return 0;
	}
	
	for ( int i = 1; i  < argc ; i++ ) 
		if ( argv[i][0] == '-' ) { 			// argument is token ( begins with - )
			if ( ! params_manager.set_variable ( act_token , act_value ) ) // [C]unable to  set a varible ... usage manual is written in params_manager
				return 1;
			act_token = argv[i]; // set new actual token 
			act_value.clear();
		}else
			act_value = argv[i];
		
		 if ( ! params_manager.set_variable ( act_token , act_value ) ) // [C]unable to  set a varible ... usage manual is written in params_manager
				return 1;

		 if ( params_manager.get_short_help() ) {
			 params_manager.print_short_help();
			 return 0;
		 }
			
	if ( ! params_manager.get_attacker_moves_first() && ! params_manager.get_defender_moves_first() ) { /*[C] correction - group status will be searched*/
			params_manager.set_attacker_moves_first( true );
			params_manager.set_defender_moves_first( true );
	}

	if ( ! ( params_manager.get_sgf_output_depth() > 0 ) ) {
	 params_manager.set_sgf_output_depth ( 0 ); // 0 means no limitations in sgf output	
	}

		
	 /* Now all program arguments are stored in adequate variables */
		
/*		if ( params_manager.get_compile_mode() ) {
		;
		} else
		{
			if ( params_manager.get_pattern_match_mode() ) {
			;	
			}
		}*/

/*Program will perform some tree searching*/

if ( params_manager.get_testing_area() ) { 
/*	Hash_table_item a;
	cout << sizeof(a);
	return 0;*/
	return 0;
}

if ( params_manager.get_multiple_pos_mode() ) { // input file is not position, but list of positions 
	int total_nodes_num = 0;
	double total_time = 0;
		string act_position_file;
		fstream mps_stream;	/*[V] stream to operate multiple positions mode*/

		/*[TD] In multiple position mode is input directory, not a list of files*/

		/*[L]*/ log_m.buff << "Multiple positions mode" << endl; log_m.flush_buffer();

		mps_stream.open ( params_manager.get_fn_pos_in().c_str() ); 
		if  ( ! mps_stream.good() ) {
			/*[L]*/ log_m.buff << " Unable to open " << params_manager.get_fn_pos_in() << endl; log_m.flush_buffer();
		return 1;
	}

	
	while ( ! mps_stream.eof() ) { 

		g_print_info = true; // it is neccessary to state that program shall print single info for every problem
			
			getline ( mps_stream ,  act_position_file ); 
		
			if ( act_position_file.empty())
				continue;
	
			cout << act_position_file << endl;

			handle_single_problem ( act_position_file , true , &total_nodes_num , &total_time );			

			cout << "====================" << endl;
			
	} // while 
		
	/*[L]*/ log_m.buff << "Final statistics:" << endl; log_m.flush_buffer_imp();
	/*[L]*/ log_m.buff << "Total nodes searched :" << total_nodes_num << endl; log_m.flush_buffer();
	/*[L]*/ log_m.buff << "Total time spend :" << total_time  << " s" << endl; log_m.flush_buffer();


	}else {

		//single position
		
	/*[L]*/ log_m.buff << "Single position mode" << endl; log_m.flush_buffer();

				handle_single_problem ( params_manager.get_fn_pos_in() , true , NULL , NULL );

	return (0);

	} // single position 

} // main()

void print_group_status ( Stone_color first_move , Group_status status ) { 
	if ( first_move == sc_black ) 
		cout << "Result of "<< g_filename << ", when attacker moves first: ";
	else
		cout << "Result of " << g_filename << ", when defender moves first: ";
		
	cout << status << endl;
	
}

/** 
 * \param[in] filename Is the name of file where the problem is saved.
 * \param[in] sgf_output Tells whether to print sgf output or not ( for instance in multiple positions mode sgf output is not printed ) 
 * \param[in] total_nodes_pt Pointer to actual number of totally searched nodes.
 * \param[in] total_time_pt  Pointer to actual amount of total time spend.
 * 
 * This function takes care of one single problem ( that means two positions : attacker moves first , defender moves first ). It also manages
 * neccessary output.
 * */
void handle_single_problem ( string filename,  bool sgf_output , int * total_nodes_pt , double * total_time_pt ) {  /*Handles everything connected with single position ( taken from params_manager )*/
	
Search_result act_result_;
Group_status status_de , status_at ; // status when defender / attacker moves first
Group_status_accuracy act_accuracy_;
double act_time;
string out_fn_base_ = get_out_fn_base ( filename ); // base of output filename
string act_filename_;

/*Initialize ,,persistent" data structures*/
			if ( params_manager.get_transposition_tables() && params_manager.get_save_tt() ) { // transposition tables and their persistency are switched on 
				t_searcher.hash_table_manager.hash_table_activate();  // hash table and hash keys for board is initialized once for both searches
				board_manager.hash_board_manager.init_hash_board();   
//				board_manager.hash_board_manager.print_hash_board();
			}

		g_filename = filename;
	
	if ( params_manager.get_attacker_moves_first() ) { // attacker moving first is demanded
		if ( sgf_output ) {
			act_filename_ = out_fn_base_ + "_at.sgf";
			sgf_printer.set_output (act_filename_);
		}

		//"Analyzing position when defender moves first"
	 	 status_at =	analyze_single_position (   filename , sc_black , &act_time ,act_filename_) ;
		 if ( total_time_pt != NULL ) 
		 (*total_time_pt ) += act_time;
		
		if ( total_nodes_pt != NULL )
				(*total_nodes_pt) += t_searcher.get_nodes_num();
		print_group_status ( sc_black , status_at );
	}

	if ( params_manager.get_attacker_moves_first() && params_manager.get_defender_moves_first() ) 
		log_m.buff << endl;

	if ( params_manager.get_defender_moves_first() ) {// defender moving first is not blocked 
		if ( sgf_output ) {
			act_filename_ = out_fn_base_ + "_de.sgf";
			sgf_printer.set_output (act_filename_);
		}

		//"Analyzing position when defender moves first"
		  status_de =	 analyze_single_position (   filename , sc_white, &act_time, act_filename_) ;
	
		 if ( total_time_pt != NULL ) 
			(*total_time_pt ) += act_time;


		if ( total_nodes_pt != NULL )
				(*total_nodes_pt) += t_searcher.get_total_nodes_num();
		print_group_status ( sc_white , status_de ) ;
	}


}

/** 
 * \param[in] pos_name is a string containing name of position to solve.
 * \param[in] color_to_move is states player starting in the position.
 * \param[in] act_time_pt is pointer to actual amount of time spend.
 * \param[in] string_filename is filename into which is output sgf saved ( in case of iddfs )
 * \return Returns result of the search. 
 * 
 * This function performs all neccessary operations for tree search in one position ( e.g. attacker moves first , defender moves first ).
 * Neccessary initializations are done before searching is started.
 * */

Group_status analyze_single_position ( string pos_name , Stone_color color_to_move, double * act_time, string save_filename) { 
			
/*Initialize data structures*/
int total_nodes = 0;
			sgf_parser.init();
			board_manager.init();
			if ( params_manager.get_transposition_tables() ) {
				if ( ! params_manager.get_save_tt() ) {  // [C] no tables storing => inicialize again
					t_searcher.hash_table_manager.hash_table_activate();
					board_manager.hash_board_manager.init_hash_board();  /*move this elsewhere ... the best into board_manager.init*/
				}
				else {
					t_searcher.hash_table_manager.hash_table_init_testing_variables(); /*[C] usually this is done in hash_table_activate ... */
					board_manager.hash_board_manager.set_position_hash(0);  /*[C] ,,reinit" position hash is neccessary*/
				}
			}else 
				board_manager.hash_board_manager.init_hash_board();  /*[C] when not using tt, this must be used though because of repetition cutoff

		/**/

			clock_t start;  
	
			start =  clock();/*[C] clock() returns number of "proccessor ticks" since beginning of the proccess ...  */


			
		if ( ! sgf_parser.parse_sgf( pos_name.c_str() ) ) {  // parses sgf input and saves it to temp_board_manager
			log_m.buff << "Sgf parsing of file : " << pos_name << " failed. Program was terminated." << endl; 
			log_m.flush_buffer_imp();
			exit ( 1 );
		}
		
		string act_comments = sgf_parser.get_temp_board_manager().get_comments();
		if ( act_comments != "" && g_print_info ) {
			log_m.buff << "Information extracted from the position:" << endl;
			log_m.buff << act_comments << endl << endl;
			g_print_info = false;
		}
		if ( color_to_move == sc_black ) {
			log_m.buff << "Analyzing position when attacker moves first" << endl; log_m.flush_buffer(); } /*[L]*/
		else {
			log_m.buff << "Analyzing position when defender moves first" << endl; log_m.flush_buffer(); } /*[L]*/

		board_manager.init_from_temp_board_manager( sgf_parser.get_temp_board_manager() ); // correctly inits board_manager from temp_board_maanger

		/*LOGGING*/ log_m.buff << "Start of tree searching" << endl; log_m.flush_buffer();
		
		Search_result result;

		t_searcher.set_ab_pruning ( params_manager.get_ab_prunning() );
		t_searcher.init();
		
		if ( params_manager.get_iddfs() ) { // iddfs activated
			result.group_status_accuracy = gsa_depth_uncertain; // only to prevent stop before any search 
			int i = 0;
			while ( ( result.group_status_accuracy != gsa_certain) ) { 
						i++;
						cout << " iteration number " << i  << " " << endl;
						switch ( color_to_move ) {
					  case sc_white: sgf_printer.set_output (save_filename);
													 result = t_searcher.search(nt_max, 0 , gs_dead, gs_alive, false,false); 
										        break;
						case sc_black: sgf_printer.set_output (save_filename);
													 result = t_searcher.search(nt_min, 0 , gs_dead, gs_alive, false,false); 
													  break;
					}
											sgf_printer.print_to_file ( ")");  // [TD] ... mofe elswhere !!! 
					/*[TU]*/		cout << " Nodes searched: " << t_searcher.get_nodes_num() << endl; //log_m.flush_buffer(); /**/
					t_searcher.increase_max_depth(); // increase maximal depth of search
			} // while 
		} // iddfs
		else  {
			switch ( color_to_move ) {
			  case sc_white: result = t_searcher.search(nt_max, 0 , gs_dead, gs_alive, false,false); break;
				case sc_black: result = t_searcher.search(nt_min, 0 , gs_dead, gs_alive, false,false); break;
			}
		t_searcher.set_total_nodes_num( result.tree_size );
		sgf_printer.print_to_file ( ")"); // ending sgf_output
		}
		
		start = clock() - start;
		double elapsed_time = ( double ) start / ( double )  CLK_TCK;  // [C] CLK_TCK == macro for number of clock_time ticks in one second
		( * act_time ) = elapsed_time;
		
		cout << " Nodes searched: " << t_searcher.get_total_nodes_num() << endl;
		cout << " Searching time :" << elapsed_time << " s"<< endl;
				
		return result.group_status; // returns only status
/**/

	
}
/**\param[in] filename Input problem filename to be transformed 
 * \return Returns transformed filename  * */
string get_out_fn_base ( string filename ) { 
	for ( int i = 0; i < filename.length(); i++) {
		if ( filename[i] == '/' || filename[i] == '.')
			filename[i] = '_';
	}

return filename;
}

 /*[TE] Ko
	 
		board_ma<F12>nager.print_stones();
		board_manager.print_blocks_info();

   board_manager.play_a_stone( sc_white , Coordinates ( 3, 7 ) );
		
		board_manager.print_stones();
		board_manager.print_blocks_info();
   
		board_manager.play_a_stone(sc_black , Coordinates ( 4, 7  ) );

		board_manager.print_stones();
		board_manager.print_blocks_info();
	/**/	
		
		
		/* [TE] illegal moves input_pos_2.sgf 
		 
			board_manager.print_stones();
			board_manager.print_blocks_info();

			board_manager.print_playable_set();

    	board_manager.play_a_stone( sc_white , Coordinates ( 7 , 7 ) );
			
			board_manager.print_stones();
			board_manager.print_blocks_info();
			board_manager.print_playable_set();
	 */
		
		/* [TE] unplaying stones input_pos.sgf 
		  while ( board_manager.unplay_node_handle != NULL ){
			board_manager.print_stones();
			board_manager.print_playable_set();
			board_manager.unplay_last_stone();
		}*/

		  /*TESTING capturing stones 
			board_manager.print_blocks_info();
			board_manager.print_playable_set();

			board_manager.unplay_last_stone();
			board_manager.unplay_last_stone();
			
			cout << " stones unplayed " << endl;
			board_manager.print_stones();
			board_manager.print_playable_set();
			board_manager.print_blocks_info();
	*/

/* TESTING function alive in eye_manager input_pos.sgf
	time_t start,end;
  double dif;

  time (&start);

		
		for ( int i = 0 ; i < 1000; i++ )  {
			;
		}
		
		time (&end);
  	dif = difftime (end,start);
		cout << dif << "seconds" << endl;
	
		Group_status status; 
		if ( status = eye_manager.is_alive(board_manager.get_point_board(), board_manager.get_playable_set()) ) 
		

		board_manager.print_stones();
		oard_manager.print_blocks_info();
	*/	

