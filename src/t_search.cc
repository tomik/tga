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

#include "t_search.h"

/**\file t_search.cc
 * \brief Contains implementation of methods related to tree searching ( \ref page_3 ).
 *
 * In this file important methods performing tree searching ( or related to
 * tree searching ) are implemented. Tree searching itself ( T_searcher::search
 * ) is done with standart minimax algorithm.
 * */


/**
 * \param[in] node_type_ Tells whether actual node is of type \c nt_min ( defender ) or \c nt_max ( attacker ).
 * \param[in] level Actual node's depth in the tree.
 * \param[in] alfa Value of alfa attribute neccessary for alfa-beta pruning.
 * \param[in] beta Value of beta attribute neccessary for alfa-beta pruning.
 * \param[in] last_move_pass Informs whether the preceeding move was pass.
 * \param[in] last_move_ko  Informs whether the preceeding move took ko for life.
 * 
 * This method ( together with Tsearcher::search_son ) provides full functionality for tree search. It implements basic minimax algorithm together 
 * with alfa beta pruning, also calls neccessary heuristics and forward pruning techniques, it also cooperates with Sgf_printer in creating proper sgf 
 * output file.
 */

Search_result T_searcher::search( Node_type node_type_, int level , Group_status alfa , Group_status beta , bool last_move_pass , bool last_move_ko) {

	Stone_color stone_color_to_move_; 
	if ( node_type_ == nt_min ) 
		stone_color_to_move_ = sc_black;
	else
		stone_color_to_move_ = sc_white;
	/**/
	
	/*[C] some intitialisations */
	if ( level == 0 ) { 
		sgf_printer.print_sgf_header(stone_color_to_move_);
		sgf_printer.print_initial_position ( board_manager.point_board);
		nodes_num = 0; // only number of nodes for actual iteration is set to 0
		
		if ( node_type_ == nt_min )   
		 uninteresting_status = gs_alive;			
		else 
		 uninteresting_status = gs_dead;			
	} 
	/**/

	bool same_depth_limit = false; // for depth inaccuracy in tt  

	int act_tree_size_ = 0; // size of actual subtree

	int total_moves_num = 0; // total number of potential moves from the node 
	int act_move_num = 0; // number of actual move

	Group_status act_status_ = gs_unknown;
	Group_status_accuracy act_accuracy_ = gsa_unknown;
	
	/*[C] lookup in transposition tables ( when transposition tables are deactivated automatically returns gs_unknown) */
	pair < Group_status , Group_status > tt_result = hash_table_manager.get_item_status ( 
																								Hash_table_item ( board_manager.hash_board_manager.get_position_hash(), stone_color_to_move_ , 
																																	board_manager.ko_manager.get_retake_coordinates(), max_depth, node_type_, 
																																	/*SWITCH OFF BITSET 
																																	board_manager.get_bitset_at() ,
																																	board_manager.get_bitset_de(),/**/ alfa, beta ) , same_depth_limit , act_accuracy_ ) ;
	
	if ( tt_result.first != gs_unknown && tt_result.first > alfa ) 
		alfa = tt_result.first;
	if ( tt_result.second != gs_unknown && tt_result.first < beta ) 
		beta = tt_result.second; 

	if ( same_depth_limit && act_accuracy_ == gsa_depth_uncertain && params_manager.get_iddfs()) 
		return Search_result ( gs_unknown , gsa_depth_uncertain , act_tree_size_ ); 
	
	if ( tt_result.first == tt_result.second ) // precise result might be gs_unknown as well
		act_status_ = tt_result.first;

	
	if ( act_status_ != gs_unknown ) {
		sgf_printer.buff << "Transposition table cutoff for status : " << act_status_ << endl ; sgf_printer.print_comment ( false );
    sgf_printer.buff << "Hash key ( normalized by color ) is: " 
										<<  hash_table_manager.get_normalized_hash_key ( 
												board_manager.hash_board_manager.get_position_hash(), stone_color_to_move_ )  << endl 
										<< "Ko retake coordinates: " << board_manager.ko_manager.get_retake_coordinates() ; sgf_printer.print_comment ( true ); 
		
		return Search_result ( act_status_ , act_accuracy_ , act_tree_size_ ); 
	}

	act_accuracy_ = gsa_certain; // it is important to set accuracy to unknown for further algorithm

	/**/
	
	/* [C] ingoing values of alfa/beta */	
	Group_status in_alfa = alfa;
	Group_status in_beta = beta; 
	/**/
	
	nodes_num++; //[C] number of searched nodes increases
	total_nodes_num++; //[C] number of searched nodes increases
	act_tree_size_ = 1;	 // [C] from now, subtree has at least one node

	if ( params_manager.get_iddfs() && level >= max_depth ) { // IDDFS bottom
		return Search_result ( gs_unknown , gsa_depth_uncertain , act_tree_size_ ); // searching reached limited depth ( with still unknown status ) => return 
	}
	/**/

	if ( level >= 100 )  { // too deep => some problem => ending search
			log_m.buff << "Total Depth cutoff ( on level 70 ), problem is too large for algorithm to bo solved, ending program ... " << endl;
			log_m.flush_buffer();
		for ( int i = 0 ; i < 101; i ++ ) 
			sgf_printer.print_to_file( ")");
	  exit(1);
	}
	/**/
	#ifdef TESTING  
		if( board_manager.ko_manager.get_active() ) { // [C] last turn was taking the ko 	
				sgf_printer.buff <<  "Ko manager is active " ; sgf_printer.print_comment(true);
	 	}	
		
	#endif
	

		sgf_printer.buff << "Ingoing alfa_beta values" << endl << "alfa : " << alfa << endl << "beta : " << beta << endl; 
		sgf_printer.print_comment(true);
	

	if ( node_type_ == nt_max ) {
		sgf_printer.buff << "This is MAX node";	
	}
	else {
		sgf_printer.buff << "This is MIN node";	 
	}
	sgf_printer.buff << endl << "Level : " << level  <<  endl ; sgf_printer.print_comment( false );
	

	sgf_printer.buff << (board_manager.print_pot_eyes_map( true )).str();sgf_printer.print_comment( true);
	/*[C] Statical life evaluation : */
	if ( (stone_color_to_move_ == sc_black && board_manager.number_stones_de < TOO_LITTLE_TO_LIVE ) || 
			 (stone_color_to_move_ == sc_white && board_manager.number_stones_de < TOO_LITTLE_TO_LIVE -1 ) )// there is too little defender's stones to live
			act_status_ = gs_dead ;
	else 
		act_status_ = eye_manager.get_group_status( board_manager.get_point_board(), board_manager.get_pot_eyes_map_pt() );

	/* [C] Staticaly discovered status is returned*/
	if ( ( act_status_ == gs_alive ) || ( act_status_ == gs_dead ) )  {
 		sgf_printer.buff << "Eye manager/capture recognition cutoff: " << endl << " status : " << act_status_ 
											<< " accuracy : " << gsa_certain << endl; sgf_printer.print_comment( false );
		sgf_printer.buff << "alfa : " << alfa << endl<< "beta : " << beta ; sgf_printer.print_comment( true );
		
		/*[C] Saving new position to transposition tables as a alfa and beta same value ( result ) is stored  */								
		hash_table_manager.store_item ( Hash_table_item ( board_manager.hash_board_manager.get_position_hash(), stone_color_to_move_, 
																											board_manager.ko_manager.get_retake_coordinates(), max_depth, node_type_, 
																											/*SWITCH OFF BITSET 
																											board_manager.get_bitset_at(), 
																											board_manager.get_bitset_de() ,/**/ in_alfa , in_beta , gsa_certain ,
																											act_status_ , act_status_ ));
			
		return Search_result ( act_status_ , gsa_certain , act_tree_size_ ); // status gained from eye manager is certain
	}

//	#ifdef TESTING 
		sgf_printer.buff << "Hashing info" << endl << "Actual position hash ( normalized by color ) is: " 
										<<  hash_table_manager.get_normalized_hash_key ( 
												board_manager.hash_board_manager.get_position_hash(), stone_color_to_move_ )  << endl 
										<< "Ko retake coordinates : " << board_manager.ko_manager.get_retake_coordinates();
		sgf_printer.print_comment( true ); /**/
//	#endif

		
	/* [C] If algorithm comes here, than the result of actual node depends on results of his sons*/
	
	Search_result son_result_; // !< status of actual son
	bool depth_uncertaincy_ = false; // !< if one of the sons is depth_uncertain this turns to true and possible uncertainty goes up the tree
	Search_result act_result_;
	bool no_move_possible = true; // !<first searched sone triggers true value
	
	bool possible_double_ko = false; // !< when set to true and ko for life appears it is a double ko 
	bool play_pass_allowed = false;   // !< true means algorithm is allowed to try pass
	bool play_ko = false;             // !< when set to true ko might be played
	int legal_moves_num = 0;          // !< number of legal moves ( self atari is a legal move, suicide is not  ) 
	Group_status after_pass_group_status = gs_unknown;  // !< status which is returned after pass move 

	Playable_vector playable_vector;
	
		
	if ( ! last_move_pass ) { /*[C] two passes are not allowed */
		playable_vector.push_back ( make_pair ( Coordinates ( -1 , -1 ) , Playable_data ( pp_pass ) ) ) ;  // [C] adding pass for playing 
		Playable_item & last_item = playable_vector.back();
		last_item.second.sync_playing_priorities(stone_color_to_move_); // [C] sets actual playing priority   
	}

	/*[C] apply heuristic and prepare playable moves together with their priorities to playable vector*/
	for ( Playable_map::iterator it = board_manager.get_playable_map().begin() ; it != board_manager.get_playable_map().end(); it++ ) {
	  if ( ( stone_color_to_move_ == sc_black && (*it).second.get_main_pp_at() == pp_none ) || 
			   ( stone_color_to_move_ == sc_white && (*it).second.get_main_pp_de() == pp_none ) ) //[C] ignoring moves with no priority
			continue;
		if (  eye_manager.full_eyes_set.find ( (*it).first ) != eye_manager.full_eyes_set.end()  &&  stone_color_to_move_ == sc_white  )  
			continue; /*[C] defender is prevented to fill his own eye*/
		
		playable_vector.push_back ( *it );
		Playable_item & last_item = playable_vector.back();

		last_item.second.sync_playing_priorities(stone_color_to_move_); /*[C] sets temp playing priorities to normal playing priorities*/
		
	/*[C] dynamic tree heuristic*/
		if ( params_manager.get_dyn_heur() && level < MAX_DYN_HEUR_DEPTH && last_item.first == dyn_heur_max[level].first ) // saved coordinates 
			if (  dyn_heur_max[level].second != -1 ) 
				last_item.second.increase_act_heuristic_pp( -1 );
		
		if ( ( board_manager.check_takes_ko( stone_color_to_move_ , last_item.first ) ).second ) { 
				play_pass_allowed = true; /*[C] when there is a attempt to play ko, pass must be tried before*/
				last_item.second.set_act_main_pp(pp_ko_last);
				sgf_printer.buff << "Ko might be taken at:"<< (*it).first; sgf_printer.print_comment( true );
				continue;		
		}
		
		if ( params_manager.get_stat_heur() ) {
		
				/*[C] self atari heuristic*/ 
				int stone_in_atari_num = 0;
				if ( ( board_manager.check_self_atari( stone_color_to_move_ , last_item.first , &stone_in_atari_num) ) ) {
					if ( stone_color_to_move_ == sc_black && stone_in_atari_num <= 2 ) 
						last_item.second.increase_act_heuristic_pp( 5 ); // attacker might sometimes selfatari with little group
					else 
						last_item.second.increase_act_heuristic_pp( 10 );
					if ( stone_in_atari_num >= 3 && stone_color_to_move_ == sc_white ) {  // SELF ATARI PRUNING
							last_item.second.set_act_main_pp ( pp_none ); // no searching when atari on more than 3 defender's stones 
						continue;
					}
				}	
							
				/*[C] capture heuristic*/
				if ( (node_type_ == nt_max ) && ( board_manager.check_capture( stone_color_to_move_ , last_item.first , NULL ) ) ) {
					last_item.second.increase_act_heuristic_pp( -3 ); /**/
				}

				int stones_num = 0; // to further differ capturing heuristic according to number of captured stones
				bool connecting_capture = false; // does a capture connect's some unsafe stones ? 
				
				/*[C] capture heuristic - for both colors */
				if ( (node_type_ == nt_min ) && ( board_manager.check_capture( stone_color_to_move_ , last_item.first , &connecting_capture ) ) ) {
					if ( connecting_capture ) 
						last_item.second.increase_act_heuristic_pp( -4 );
					else 
						last_item.second.increase_act_heuristic_pp( -3 );
				}

				
				/*[C] atari heuristic*/
				if ( ( node_type_ == nt_max ) && ( board_manager.check_atari( stone_color_to_move_ , last_item.first ) ) ) {
					last_item.second.increase_act_heuristic_pp( -2 );
				}
				
				/*[C] atari heuristic */
				if ( (node_type_ == nt_min ) &&  ( board_manager.check_atari( stone_color_to_move_ , last_item.first ) ) ) { 
					last_item.second.increase_act_heuristic_pp( -2 );
				}
				
				/*[C] take away liberty heuristic*/
				if ( (node_type_ == nt_max ) && ( board_manager.check_take_liberty( stone_color_to_move_ , last_item.first , 0 ) ) ) {
					last_item.second.increase_act_heuristic_pp( -1 );
				}
				
				/*[C] take away liberty heuristic*/
				if ( (node_type_ == nt_max )  && ( board_manager.check_take_liberty_of_safe_attacker ( stone_color_to_move_ , last_item.first ) ) ) {
					last_item.second.increase_act_heuristic_pp( 1 );
				}
				
				/*[C] check connection */
				if ( (node_type_ == nt_max ) &&  ( board_manager.check_connection( stone_color_to_move_ , (*it).first , false) ) ) {
					last_item.second.increase_act_heuristic_pp( -1 ); /**/
				}
				
				/*[C] connection heuristic*/
				if ( (node_type_ == nt_min ) && ( board_manager.check_save_connection( stone_color_to_move_ , (*it).first ) ) ) {
					last_item.second.increase_act_heuristic_pp( -5 );
				}
				
				/*[C] check connection of larger blocks ( important for under the stones ) */
				if ( (node_type_ == nt_max ) &&  ( board_manager.check_connection( stone_color_to_move_ , (*it).first , true ) ) ) {
					last_item.second.increase_act_heuristic_pp( -5 ); /**/
				}
	
		}
		
		/*[C] if it starts ko it will be searched as a last move == pp_ko_last*/
		
	} // for 
		

	/*[C] sort orders moves from ,, lowest " numbers to highest ... -> therefore
	 * bigger priority means lesser number ( adding -1 to preffered moves in
	 * heuristics)*/
	sort ( playable_vector.begin() , playable_vector.end() ) ;
		
	/*OUTPUT*/
	for ( Playable_vector::iterator it = playable_vector.begin() ; it != playable_vector.end(); it++ ) {
		sgf_printer.buff << (*it);
	}	
	sgf_printer.print_comment(true);
  /**/
	
	/*[C] cycle through sons*/
	for ( Playable_vector::iterator it = playable_vector.begin() ; it != playable_vector.end(); it++ ) {
		
		if ( (*it).second.get_act_main_pp() == pp_none ) // skipping moves with "none" actual priority
			continue;
	
		bool play_ko_for_life = false;  // !<  is this move a ko for life ( no other move or passing works )
		bool pass = false; // !< this move is pass, must be here to ,,rearange after every move"

		/*[C] Managing pass moves*/
		if ( Coordinates  (-1,-1) == (*it).first ) {
			pass = true;
	
		if ( stone_color_to_move_ == sc_white ) { 
			/*[C] it is useless to try pass when group is alive, pass is tried only when remain <=4 legal moves ( forward pruning technique) */ 
			if ( act_status_ != gs_alive  && legal_moves_num <= 4 ) // TODO && at least 2 unsafe attacker's exist !!!
			play_pass_allowed = true;
		}
		else /*attacker's node - if he can get a ko and pass brings death to white he is dead*/
			if ( act_status_ == gs_ko  && legal_moves_num <= 4 ) 
			play_pass_allowed = true;
		}

		switch ( node_type_) { 
		
		case nt_max :  /*defender*/

			/*[C] pass is played only when play_pass_allowed is set to true */
			if ( pass )  
				if ( play_pass_allowed) {
					sgf_printer.buff << "trying pass "; sgf_printer.print_comment( true );
				}
				else
					continue; 
			
			/*[C] playing a stone*/
			if ( ! board_manager.play_a_stone ( sc_white ,(*it).first ) ) 
				continue; 
			else
				legal_moves_num++; /*[C] move is legal even though it might be stupid*/
			
			/* [C] ko handling*/
			if ( ( params_manager.get_ko_handling() ) && board_manager.ko_manager.get_active() ) { // [C] defender took ko 
					play_ko = true;
				
			}
			/**/

			/*[C] Play ko for life means that to play this ko is defender's last hope ( it must be examined whether he lives afterwards)*/
			if ( play_ko 
								&& after_pass_group_status != gs_unknown  // [C] if gs_unknown -> no determined status was gained after pass -> unc. alive
								&& after_pass_group_status <= gs_ko	   // [C] when white ignore black threat he might get at least ko, otherwise unc.alive  
								&& ( act_status_ !=  gs_alive ) ) // [C] defender cannol live without ko 
				play_ko_for_life = true;
			
			no_move_possible = false; // move was made

/*			if ( play_ko && ! play_ko_for_life ) {
				board_manager.unplay_last_stone();
				continue;
			}*/
			/*[C] searching the son and misc.*/
			son_result_ = search_son ( (*it).first , nt_min , level , alfa , beta , play_ko_for_life);
			
			act_tree_size_+= son_result_.tree_size; // addinf size of the son's tree to actual tree size

			if ( son_result_.group_status_accuracy == gsa_depth_uncertain ) { // depth uncertain status mean it might be everything -> no information value
				depth_uncertaincy_ = true;
				son_result_.group_status = gs_unknown;
			}
				
			if ( son_result_.group_status == gs_unknown ) {
				board_manager.unplay_last_stone();
				continue;
			}
			
			/*[C] pass was played and result is known => store result */
			if ( pass  ) {
				after_pass_group_status = son_result_.group_status;
				if ( son_result_.group_status == gs_ko ) 
					son_result_.group_status = gs_dead;
			}
			/**/
			
			/* [C] ko handling*/
			if ( ( params_manager.get_ko_handling() ) && (play_ko_for_life && son_result_.group_status == gs_alive )) { 
					son_result_.group_status = gs_ko;
				}
			/**/

			/* [C] double ko handling, ko was taken this turn - possible double ko check  */
			if ( ( params_manager.get_ko_handling() ) && (play_ko_for_life && son_result_.group_status == gs_ko )) { 
				if ( possible_double_ko ) { /*[C] two independent ko's which white can start -> life*/
						sgf_printer.buff <<  "Alive in double ko" ; sgf_printer.print_comment(true);
					son_result_.group_status = gs_alive;
				}
				else 
					possible_double_ko = true;
				if ( last_move_ko ) { /*[C] last move started ko for life of the group and defender found another one to survive => double ko*/
					son_result_.group_status = gs_alive;
					sgf_printer.buff <<  "Alive in double ko" ; sgf_printer.print_comment(true);
				}
			}


			if (  son_result_.group_status != gs_unknown && ( act_status_ <= son_result_.group_status  || act_status_ == gs_unknown )) {
				
				if ( act_status_ == son_result_.group_status ) {
					if ( act_accuracy_ != gsa_certain )  // this is to substitute actual accuracy with better accuracy ( certain )
						act_accuracy_ = son_result_.group_status_accuracy;
				}
				else  // act_status_ < son_result_.group_status
					act_accuracy_ = son_result_.group_status_accuracy; 
			
				act_status_ = son_result_.group_status; 
						
				if ( ab_pruning && alfa < act_status_)  // alfabeta pruning
					alfa = act_status_;
			}

			/*[C] alfa beta cutoff*/
			if ( ab_pruning && ( beta <= act_status_ ) ) { //Pruning
				if ( act_status_ == gs_alive ) 
					act_accuracy_ = gsa_certain; // status is dead ( even if further search is done ) 
				else 
					act_accuracy_ = gsa_alfa_beta_uncertain; // status might change ( but i can alfa beta cutoff without infomation loss ) 
				
			 /*OUTPUT*/ sgf_printer.buff << " status : "<< act_status_<< endl << 
				 															" accuracy :" << act_accuracy_ << endl; sgf_printer.print_comment( false );
				  	 	 	 	 sgf_printer.buff << " alfa : " << alfa << endl << " beta : " << beta <<  endl <<  endl ;  sgf_printer.print_comment( true ); /**/
						 
				/*OUTPUT*/		sgf_printer.buff << "Other Variations were alfa-beta prunned ";	sgf_printer.print_comment( true ); /**/
				board_manager.unplay_last_stone(); //unplays son's move 
				

				if ( act_status_ != gs_unknown ) 
					hash_table_manager.store_item ( Hash_table_item ( board_manager.hash_board_manager.get_position_hash(), stone_color_to_move_,
																														board_manager.ko_manager.get_retake_coordinates(), max_depth, node_type_, 
																														/*SWITCH OFF BITSET 
																														board_manager.get_bitset_at() ,
																														board_manager.get_bitset_de(), /**/ in_alfa , in_beta , act_accuracy_ , 
																														act_status_ , act_status_ ));

					return Search_result ( act_status_ , act_accuracy_ , act_tree_size_ ); // alfa beta estimate is returned
			} // pruning
				break;
								
	case nt_min :  /*attacker */

			/* in this case are many things which are same in nt_max, therefore i won't comment them here*/
							
			#ifdef TESTING
				sgf_printer.buff << "Actual position hash is: " 
																				<<  board_manager.hash_board_manager.get_position_hash(); sgf_printer.print_comment( true ); /**/
			#endif

			/**/	
			if (  pass )  
				if ( play_pass_allowed ) {
					sgf_printer.buff << "trying pass "; sgf_printer.print_comment( true );
				}
				else
					continue; 
			/**/
			
			if ( ! board_manager.play_a_stone (  sc_black,(*it).first ) ) 
				continue; // move was illegal => stone was not played
			

			/* [C] ko handling*/
			if ( ( params_manager.get_ko_handling() ) && board_manager.ko_manager.get_active() ) { // [C] attacker took ko 
					play_ko = true;
				
			}
			
			/**/
			if ( play_ko 
					&&  ( after_pass_group_status != gs_unknown ) /*[IMP] gs_unknown -> no determined status was gained after pass -> unc. alive*/ 
					&&  ( after_pass_group_status >= gs_ko)  /*[IMP] when black ignores threat defender might get at least ko no unconditional death*/ 
					&& ( act_status_ != gs_dead ) /*  && ( act_status_ != gs_ko ) /*TODO*/ )
				play_ko_for_life = true;
			
			/*if ( play_ko && ! play_ko_for_life ) {
				board_manager.unplay_last_stone();
				continue;
			}*/

			if ( play_ko_for_life && act_status_ == gs_ko ) { // Best attacker can get from playing a ko is a ko ( and he already has it ) -> skipping this move
				board_manager.unplay_last_stone();
				continue;
			}
			
			no_move_possible = false; // a legal move was made there will be no "no move possible" handling   
			
			/*[C] searching the son and misc.*/
			son_result_ = search_son ( (*it).first , nt_max , level , alfa , beta , play_ko_for_life);
			
			act_tree_size_+= son_result_.tree_size; // addinf size of the son's tree to actual tree size
			
			if ( son_result_.group_status_accuracy == gsa_depth_uncertain ) { // depth uncertain status mean it might be everything -> no information value
				depth_uncertaincy_ = true;
				son_result_.group_status = gs_unknown;
			}
			
			if ( son_result_.group_status == gs_unknown ){
				board_manager.unplay_last_stone();
				continue;
			}

			if ( son_result_.group_status == gs_ko && last_move_pass ) { // this is obvious transformation ... 
				son_result_.group_status = gs_dead;
			}

			/*[C] store result after pass*/
			if ( pass) {
				after_pass_group_status = son_result_.group_status;
//				if ( son_result_.group_status == gs_ko ) 
//					son_result_.group_status = gs_dead;
			}
			

			/* [C] ko handling*/									
			if ( ( params_manager.get_ko_handling() ) && (play_ko_for_life && son_result_.group_status == gs_dead ) ) {
					son_result_.group_status = gs_ko;
			}
			/**/


			if ( son_result_.group_status != gs_unknown && ( son_result_.group_status <= act_status_ || act_status_ == gs_unknown ) ) {
			
				if ( act_status_ == son_result_.group_status ) {
					if ( act_accuracy_ != gsa_certain )  // this is to substitute actual accuracy with better accuracy ( certain )
						act_accuracy_ = son_result_.group_status_accuracy;
				}
				else  // act_status_ > son_result_.group_status
					act_accuracy_ = son_result_.group_status_accuracy; 

				act_status_ = son_result_.group_status;

				if ( ab_pruning && act_status_ < beta )  // alfabeta pruning
					beta = act_status_;
			}
			
			if ( ab_pruning && ( act_status_ <= alfa ) ) {
				if ( act_status_ == gs_dead ) 
					act_accuracy_ = gsa_certain; // status is dead ( even if further search is done ) 
				else 
					act_accuracy_ = gsa_alfa_beta_uncertain; // status might change ( but i can alfa beta cutoff without infomation loss ) 

				/*OUTPUT*/ sgf_printer.buff << " status : "<< act_status_ << endl << " alfa : "	
																		 << alfa << endl << " beta : " << beta << endl << " accuracy :" << act_accuracy_ << endl; /**/
									 sgf_printer.print_comment( false);
									 sgf_printer.buff << "Other Variations were alfa-beta prunned ";	sgf_printer.print_comment( true ); /**/
				board_manager.unplay_last_stone(); // unplay son's move
				
				if ( act_status_ != gs_unknown ) 
					hash_table_manager.store_item ( Hash_table_item ( board_manager.hash_board_manager.get_position_hash(), stone_color_to_move_,
																														board_manager.ko_manager.get_retake_coordinates(), max_depth, node_type_ , 
																														/*SWITCH OFF BITSET 
																														board_manager.get_bitset_at() ,
																														board_manager.get_bitset_de(),/**/ in_alfa , in_beta , act_accuracy_, 
																														act_status_ , act_status_ )); 
				  return Search_result ( act_status_ , act_accuracy_ , act_tree_size_ );   
			}	
			break;
		} // switch
 
		/*[C] unplaying stone after son was searched*/
		board_manager.unplay_last_stone(); 

	} //for ( going through sons)


	/*[C] not even pass was played */
	if ( no_move_possible ) { 

		/*[C] Try eye recognition*/ 
		act_status_ = eye_manager.get_group_status( board_manager.get_point_board(), board_manager.get_pot_eyes_map_pt() ); 
	
		if ( ( act_status_ == gs_unknown ) )  {   /*[C] eye status is unknown*/

			#ifdef TESTING	
				sgf_printer.buff << "No move possible" ; sgf_printer.print_comment ( true );
			#endif
			if ( node_type_ == nt_max ) {
						board_manager.play_a_stone ( sc_white,  Coordinates ( -1 , -1 ) ); /*done only for ko reactivation*/
						act_result_ = search_son ( Coordinates ( -1 , -1 ) , nt_min , level  , alfa , beta , false ); /*false means no ko */
						
						act_tree_size_ = act_result_.tree_size; // adding a son's tree size to actual tree size
							act_status_ = act_result_.group_status;
							if ( act_result_.group_status_accuracy == gsa_depth_uncertain ) {
								depth_uncertaincy_ = true;
								act_status_ = gs_unknown;
							}
						board_manager.unplay_last_stone();
					
					if ( act_status_ == gs_dead || act_status_ == gs_ko  ) /*[C] even ko means death ( attacker might wait to the end and then cover all ko threats and kill the group)*/
						act_status_ = gs_dead;
			}
			else
				act_status_ = gs_alive;
		} // if act_status == gs_unknown
	}  // no_move_possible

	/*[C] this is performed when all sons were searched and no cutoff was done*/

	if ( depth_uncertaincy_ ) 
		act_accuracy_ = gsa_depth_uncertain;
	else
		; // if it comes here act_accuracy_ is already counted 
	
	/*OUTPUT*/ sgf_printer.buff << " status : "<< act_status_<< endl  << 
																 " accuracy :" << act_accuracy_ << endl; sgf_printer.print_comment( false );
						 sgf_printer.buff << " alfa : " << alfa << endl << " beta : " << beta <<  endl <<  endl ;  sgf_printer.print_comment( true ); /**/
						 
	if ( act_status_ != gs_unknown && ! depth_uncertaincy_ ) // certain status 
		hash_table_manager.store_item ( Hash_table_item ( board_manager.hash_board_manager.get_position_hash(), stone_color_to_move_,
																											board_manager.ko_manager.get_retake_coordinates(),max_depth, node_type_ , 
																											/*SWITCH OFF BITSET 
																											board_manager.get_bitset_at() ,
																											board_manager.get_bitset_de() ,/**/in_alfa, in_beta , act_accuracy_ ,
																											act_status_ , act_status_ )); 
	else // uncertain status
		hash_table_manager.store_item ( Hash_table_item ( board_manager.hash_board_manager.get_position_hash(), stone_color_to_move_,
																										board_manager.ko_manager.get_retake_coordinates(),max_depth, node_type_ ,
																											/*SWITCH OFF BITSET 
																											board_manager.get_bitset_at() ,
																											board_manager.get_bitset_de() ,/**/ in_alfa, in_beta , act_accuracy_, 
																									  	alfa , beta )); 
	// returning ordinary status ( not alfa beta pruned ) 	
		return Search_result ( act_status_ , act_accuracy_ , act_tree_size_ ); 


} // search 

/**
 * \param[in] node_type_ Tells whether actual node is of type \c nt_min ( defender ) or \c nt_max ( attacker ).
 * \param[in] coordinates_ Which move is to be tried.
 * \param[in] level Actual node's depth in the tree.
 * \param[in] alfa Value of alfa attribute neccessary for alfa-beta pruning.
 * \param[in] beta Value of beta attribute neccessary for alfa-beta pruning.
 * \param[in] ko_for_life  If set to \c true then move was taking a ko and is a possible ko for life 
 * \result Returns result of the search in the son.
 * 
 * This method's main purpose is to call T_searcher::search method ( with given parametres). Thus these two methods are in 
 * undirect recursion relationship. Other tasks done within this function are : repetition cutoff, adequate sgf printing, ... */

Search_result T_searcher::search_son ( Coordinates coordinates_ , Node_type node_type_ , int level_ , Group_status alfa , Group_status beta, bool ko_for_life) { 

	/*ko_for_life says if the move played is ko for life and is handed to the son */
	
	bool repetition_cutoff = false; // !< possible repetition cutoff
	Search_result son_result_ = Search_result ( gs_unknown , gsa_certain, -1 );
	Search_result act_result_;
	unsigned long pos_in_stream;  // !< used for sgf output prunning
	bool pass_ = false; // !< is the move pass ? 

	string color_to_write_char; // !< color to move to be written to the output
	Stone_color color_to_move;

	if ( node_type_ == nt_min ) { /*[C] => son is nt_min*/
		color_to_write_char = "W"; // writing actual color 
		color_to_move = sc_black;
	}
	else {
		color_to_write_char = "B";
		color_to_move = sc_white;
	}
		

	if ( Coordinates ( -1 , -1 ) == coordinates_ ) 
		pass_ = true;

	/*[C] repetition check  */
	if (  !hash_repset_manager.store_item ( hash_table_manager.get_normalized_hash_key( 
																			 board_manager.hash_board_manager.get_position_hash() , color_to_move ) ) ) { //[C] repetition
			repetition_cutoff = true;
	} /**/

	/* [C] printing and searching the son*/
	pos_in_stream=sgf_printer.sgf_out.tellp();

	if ( ! ( pass_ )) // PASS
		sgf_printer.print_to_file ( "MA[" + sgf_printer.coordinates_to_string( coordinates_) + "]" );		 /*[C] Marks a coordinate to move*/
	sgf_printer.print_to_file ( "\n(;" + color_to_write_char + "[" + sgf_printer.coordinates_to_string( coordinates_) + "]" );  /*[C] Makes a move*/
	if ( ! ( pass_ )) // PASS
		sgf_printer.print_to_file ( "CR[" + sgf_printer.coordinates_to_string( coordinates_) + "]" );		 /*[C] Marks a made move*/
	if ( board_manager.ko_manager.get_active() )
		sgf_printer.print_to_file ( "TR[" + sgf_printer.coordinates_to_string( board_manager.ko_manager.get_retake_coordinates() ) + "]" );		 /*[C] ko*/

	
		if ( repetition_cutoff ) {
			sgf_printer.buff << "Repetition cutoff."; sgf_printer.print_comment ( false ); // OUTPUT
			son_result_.group_status = gs_unknown;
//			hash_repset_manager.remove_item ( hash_table_manager.get_normalized_hash_key( /*[C] after search pos. must be removed*/ 
																		 //board_manager.hash_board_manager.get_position_hash() , color_to_move ) );
		}
		else { /*[C] no repetition ... search the son and then remove repetition item*/
			son_result_ = search ( node_type_ , level_ + 1, alfa , beta, pass_, ko_for_life );

			hash_repset_manager.remove_item ( hash_table_manager.get_normalized_hash_key( /*[C] after search pos. must be removed*/ 
																		 board_manager.hash_board_manager.get_position_hash() , color_to_move ) );
		}
		

		int heur_tree_size;
		
		if ( ( node_type_ == nt_min && son_result_.group_status != gs_dead ) || ( node_type_ == nt_max && son_result_.group_status != gs_alive ) )
			if ( son_result_.group_status != gs_ko ) 
				heur_tree_size = 2;
			else 
				heur_tree_size = 1;
		else 
			heur_tree_size = 0;

		add_tree_size_to_dyn_heur ( level_ ,  coordinates_ , heur_tree_size );
		
	sgf_printer.print_to_file ( ")" );
		
	if (  (  ! params_manager.get_sgf_output_depth() == 0 ) && ( level_ >= params_manager.get_sgf_output_depth() ) ) {
				sgf_printer.sgf_out.seekp(pos_in_stream);										
		}

	/*[C] sgf output pruning*/
	if ( params_manager.get_pruned_sgf_output() && ( son_result_.group_status == uninteresting_status || son_result_.group_status_accuracy != gsa_certain ))  
		sgf_printer.sgf_out.seekp(pos_in_stream);										
	/**/ 
	
 	return Search_result ( son_result_.group_status , son_result_.group_status_accuracy , son_result_.tree_size );
	
}

/**
 * \param[in] stone_color_to_move_ Represents a player who is to make a first move.
 * 
 * Mehthod prints neccessary sgf header to the sgf output.
*/

void Sgf_printer::print_sgf_header(Stone_color stone_color_to_move_ ) { // TODO ... different board sizes

stringstream act_str;


	if ( stone_color_to_move_ == sc_black ) 
		act_str << "(;GM[1]FF[4]CA[UTF-8]AP[CGoban:2]ST[2]RU[Japanese]SZ[" << B_HEIGHT << "]KM[0.00]PL[B]";  // PL[W] == Black starts
	else
		act_str << "(;GM[1]FF[4]CA[UTF-8]AP[CGoban:2]ST[2]RU[Japanese]SZ["<< B_HEIGHT << "]KM[0.00]PL[W]";

	print_to_file ( act_str.str() );
	
	
}

/**
 * \param[in] coordinates_ Input coordinates.
 * \return[out] Returns string value for sgf output of given coordinates.
 * 
 * Input coordinates are transformed to string which might be directly printed into the sgf  output. Pass move is handled properly.
*/

string Sgf_printer::coordinates_to_string ( Coordinates  coordinates_ ) { 
	string str="";
	if ( coordinates_ == Coordinates ( -1 , -1 ) ) 
		return "tt"; // pass
	str = 'a' + coordinates_.get_co_column();
	str+= 'a' + coordinates_.get_co_row();
	
	return str;
}

/** 
 * \param[in] str Input string to be printed.
 * Prints given string directly to the output file.
*/

void Sgf_printer:: print_to_file( string str ) {
		sgf_out << str;
}

/** 
 * \param[in] point_board Reference to board_manager's point_board.
 * 
 * Prints initial position given as a point_board. 
*/
		
void Sgf_printer::print_initial_position ( Point_board & point_board ) { 
	for ( int i = 0; i < B_HEIGHT ; i++ ) 
		for ( int j = 0; j < B_WIDTH ; j++) { 
			switch ( point_board[i][j] ) {   /*Setting up a position with adequate sgf tags*/
					case ps_uns_at:   print_to_file( "AB[" ); print_to_file (  coordinates_to_string ( Coordinates ( i , j ) ) + "]" ) ; break;
					case ps_at:			  print_to_file( "AB[" ); print_to_file (  coordinates_to_string ( Coordinates ( i , j ) ) + "]" ) ; break;
					case ps_de:			  print_to_file( "AW[" ); print_to_file (  coordinates_to_string ( Coordinates ( i , j ) ) + "]" ) ; break;
			} //switch
	  } // for 
}

/** 
 * \param[in] blockable Blockable switcher.
 * 
 * Prints content of Sgf_printer::buff into the sgf output in a form of comment ( adds comment tags ). 
 * If blockable switcher is set to true and \c -to option is not given, than Sgf_printer::buff is cleared and no output is generated :
 * this is to distinguish important output ( might not be blocked ) from the extended .
*/

void Sgf_printer::print_comment ( bool blockable ) { 

	stringstream s;
	
	if ( ! params_manager.get_sgf_testing_output() && blockable ) { /*[C] No testing == No comments*/
		buff.str("");
		return;
	}
	
	s << "C[" << buff.str() << endl << "]"; print_to_file ( s.str()); s.str("");
	s << "C[===========================" <<  endl << "]"; print_to_file ( s.str() ); s.str("");
	
	buff.str("");
	
}

/** 
 * Activates ( and initializes ) hash_table which works as transposition tables ( saves position ). 
*/

void Hash_table_manager::hash_table_activate () { 
	active = true;
	collision_num=0;
	longest_row=0;
	items_num = 0;

	hash_table_size_log = 21;  // log (2) hash_table_size
	hash_table_size = 2 * 1048576;  // 1048576 == 2^20,  TODO ... should be adjustable by options 
	
	shift_num = hash_key_length - ( hash_table_size_log );  // upper bits in hash key are ,,more random" than lower bits => we take upper
	if ( shift_num < 0 ) 
		shift_num = 0;

	if ( hash_key_length != EXPECTED_HASH_KEY_LENGTH ) {
		log_m.buff << "Different hash_key_length than expected, program might not work well. Hash_key_length : "  
							 << hash_key_length << ", Expected : " << EXPECTED_HASH_KEY_LENGTH << endl; log_m.flush_buffer_imp();
	}
	
	at_to_move_normalisation = get_random_Hash_key();  // [C] neccessary, before searching in hash table each key is normalised with color to move
	de_to_move_normalisation = get_random_Hash_key();
	
	if ( hash_table != NULL  ) 
		delete ( hash_table);
	
	/*[C] hash table initialization*/
	hash_table = new Hash_table_item *  [hash_table_size]; 

	for ( long i = 0; i < hash_table_size; i++ ) 
		hash_table[i] = NULL;
}

/** 
 * \param[in] hash_key_ Hash key of the actual position.
 * \param[in] stone_color_to_move_ Which player is to move.
 * Activates ( and initializes ) hash_table which works as transposition tables ( saves position ). 
*/

Hash_key Hash_table_manager::get_normalized_hash_key ( Hash_key hash_key_ , Stone_color stone_color_to_move_ ){
	if ( stone_color_to_move_ == sc_black ) 
		return hash_key_ ^ at_to_move_normalisation;
	else
		return hash_key_ ^ de_to_move_normalisation;
};

/**
 * \param[in] item Hash_table_item which is about to be stored.
 * 
 * Method takes hash key from given item ( normalizis it ) and then finds adequate position in the hash table, if the position is full it goes
 * along the dynamic list in the position and stores actual item in the end.
 */

void Hash_table_manager::store_item( Hash_table_item item ) {
bool no_more_space = false;
Hash_table_item * replace_item = NULL ;
  if ( ! active ) 
		return;

	if ( items_num > (int ) hash_table_size / 2 ) { // too many items => it is impossible to store new items 
		///cout << "A" << endl;
		bool no_more_space = true;
	}

	/*[C] normalization is neccessary*/
	item.hash_key = get_normalized_hash_key ( item.hash_key , item.stone_color_to_move);
	
	/*[C] primary hashing function is simple modulo on shifted key ( because upper bits of the key are supposed to be "more random") */
	unsigned long pos = ( item.hash_key >> shift_num ) % ( hash_table_size );
	
	if ( hash_table[pos] != NULL ) {
		collision_num++;
		
		/*TESTING log_m.buff << "COLLISION number " << collision_num << " at " << pos << endl;/**/

		int act_row;
		Hash_table_item * act_item = hash_table[pos];
	
		if ( no_more_space && act_item != NULL ) {
			item.next = act_item->next;
			(*act_item ) = item;
			return;
		}
		/*[C] goes through the list in the position*/
		for ( act_row = 1 ; act_item->next != NULL ; act_row++ ) {
			if (  actualize_item (  act_item , item ) ) 
				return; // successfull actualization
			act_item = act_item->next;
		}
		act_item->next = new Hash_table_item ( item ); // placing actual position to the end of the chain

		if ( act_item->next == NULL ) // No more disk space 
			return;
			
		items_num++;
		
		if ( act_row > longest_row ) 
			longest_row = act_row;
	
		return;
	}
	/*TESTING log_m.buff << "Successfull storing at : " << pos << " , hash key : " << item.hash_key << " ,color :" <<  item.stone_color_to_move << endl;
							log_m.flush_buffer();/**/
	
	/*[C] saving position when there is no collision*/
	hash_table[pos] = new Hash_table_item ( item );
	items_num++;
}

/**
 * \param[in] item Hash_table_item which is about to be found in the Hash_table_manager::hash_table.
 * \param[out] same_depth_limit returns true if a stored position was stored in same iteration ( with same depth limit ) like incoming position 
 * \param[out] accuracy_ returns Group_status_accuracy of stored position
 * \return Returns pair of coordinates representing new alfa and new beta.
 * 
 * Method takes hash key from given item ( normalizis it ) and then if there exists item with the same hash_key ( normalized ) in the 
 * Hash_table_manager ( which is tested by funciton ::corresponds similar to operator==) it returns its stored group status. 
 */

pair < Group_status, Group_status > Hash_table_manager::get_item_status( Hash_table_item item , bool & same_depth_limit , Group_status_accuracy & accuracy_ ) {
	same_depth_limit = false;
	if ( ! active ) 
		return make_pair ( gs_unknown, gs_unknown ) ;

	item.hash_key = get_normalized_hash_key ( item.hash_key , item.stone_color_to_move);
	unsigned long pos = ( item.hash_key >> shift_num ) % ( hash_table_size );
	
	Hash_table_item * act_item_ = hash_table[pos];
	while ( act_item_ != NULL ) {
		if ( corresponds ( (*act_item_),  item ) ) {
			if ( item.depth == (*act_item_).depth )
  				same_depth_limit = true;
			accuracy_ = (*act_item_).accuracy;
			return make_pair( act_item_->new_alfa, act_item_->new_beta); 
		}
		
			act_item_ = act_item_->next;
	}
	/*[C] position was not found:*/
	return make_pair ( gs_unknown,gs_unknown);
}

/**
 *Method prints simple info about Hash_table_manager.
 */

void Hash_table_manager::print_info () { 
log_m.buff << "This is Hash_table_manager info" << endl; log_m.flush_buffer_imp();
log_m.buff << "Collision number: " << collision_num << endl;
log_m.buff << "Longest row :		 " << longest_row << endl; 
log_m.flush_buffer();
}

bool Hash_table_manager::actualize_item( Hash_table_item * saved_item, Hash_table_item & new_item ) {
	if  ( ( (*saved_item).hash_key == new_item.hash_key ) ) { 
		if ( ! ( (*saved_item).ko_coordinates == new_item.ko_coordinates ) ) {
			return false;
		}

		if  ( ( (*saved_item).alfa >= new_item.alfa )  && 
					( (*saved_item).beta <= new_item.beta ) ) {

		(*saved_item).alfa = new_item.alfa;
		(*saved_item).alfa = new_item.alfa;
		(*saved_item).new_alfa = new_item.new_alfa;
		(*saved_item).new_beta = new_item.new_beta;
		(*saved_item).accuracy = new_item.accuracy;
		return true;
		}

	}
	return false;
}
	
/**
 *\param[in] saved_ Reference to Hash_table_item which was saved in the Hash_table_manager::hash_table.
 *\param[in] new_ Reference to Hash_table_item which corressponds to actual position.
 
 * Method tests if it is possible to use stored information ( Group_status ) of saved_ Hash_table_item for actual position
 * represented by new_ Hash_table_item. It's similar to operator==.
 */

bool corresponds (  Hash_table_item & saved_, Hash_table_item & new_ ) { 
	if  ( ( saved_.hash_key == new_.hash_key ) ) { /*[C] stone color is included in key*/
		if ( ! ( saved_.ko_coordinates == new_.ko_coordinates ) ) {
		/*TESTING	cout << "Different ko coordinates at same position. Hash key: " << saved_.hash_key << endl; /**/
			return false;
		}
		//if ( ( saved_.new_alfa == saved_.new_beta ) &&  ( saved_.new_alfa != gs_ko ) ) {
		if ( saved_.accuracy == gsa_certain ) { //certain status provides more general condition to apply transposition table cutoff
			if ( ! ( ( ( saved_.node_type == nt_max ) && ( ( new_.beta <= saved_.beta ) || ( saved_.new_alfa < saved_.beta )  ) )  || 
	  				 ( ( saved_.node_type == nt_min ) && ( ( saved_.alfa <= new_.alfa ) || ( saved_.alfa < saved_.new_beta ) ) ) ) ) 
			/*[C] this is neccessary for keeping search consistency*/
				return false;
		}
		else
			if ( ! ( ( new_.beta <= saved_.beta ) && ( saved_.alfa <= new_.alfa ) )  ) 
				return false;
			
			
		/* SWITCHED OFF BITSET 
		if 		( ! ( ( saved_.bitset_at ^ new_.bitset_at).none() )  && ( ( saved_.bitset_de ^ new_.bitset_de).none() ) ) 
		cout << "PROBLEM, two different positions produced same hash_key : " << saved_.hash_key 
		<< ", probability of this error : around 1 / ( 2 ^ 60 ) :] " << endl;
		/**/
		
	return true;
}
return false;
		
}

/**
 * \param[in] level_ Actual level in the subtree.
 * \param[in] coordinates_ Coordinates of the move whose subtree size is going to be stored.
 * \param[in] tree_size_ Stored tree_size.
 *
 * This function creates information for using tree size heuristic. It fills information about how big subtrees where under different moves in 
 * different levels of searched tree so far.*/

void T_searcher::add_tree_size_to_dyn_heur ( int level_ , Coordinates coordinates_ , int tree_size_ ) { 
	
	if ( level_ >= MAX_DYN_HEUR_DEPTH ) 
		return;

	Dyn_heur_map::iterator it = dyn_heur[level_].find ( coordinates_ );
	int act_max_ = dyn_heur_max[level_].second;
	int new_value_ = -1; // new_value of the move at coordinates_

	
	if ( it == dyn_heur[level_].end() ) { // coordinate was not in the map 
//		cout << "adding " << coordinates_ << "to level " << level_ << endl;
		dyn_heur[level_].insert ( make_pair ( coordinates_ , tree_size_ ) );
		new_value_ = tree_size_;
	} else  { // coordinate already in the map 
		(*it).second += tree_size_;  
		new_value_ = (*it).second;
	}

	if ( act_max_ == -1 || new_value_ > act_max_ ) 
		set_dyn_heur_max ( level_ , coordinates_ , new_value_ );
		
}


ostream & operator<< ( ostream & out, Group_status_accuracy A ) {
	switch (A) { 
		case gsa_certain : out << "certain"; break;
		case gsa_depth_uncertain : out << "depth uncertain"; break;
		case gsa_alfa_beta_uncertain : out << "alfa beta uncertain"; break;
	}
	return out;
}
