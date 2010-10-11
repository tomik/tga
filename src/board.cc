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

/**\file board.cc
 * \brief Contains implementation of methods related to position presentation ( \ref page_6 ).
 *
 * In this file important methods performing playing and unplaying move are implemented, together with interface 
 * neccessary for hashing positions. Approach to position presentation is block oriented.
 */

#include "board.h"

/**\return Returns random hash key.
 * This function is used for filling Hash_board_manager::hash_board with random hash keys ( as defined in Zobrist hashing)*/
Hash_key get_random_Hash_key() {
	return (Hash_key) ( ( (double ) rand() / ( (double) ( RAND_MAX) + (double) (1) ) ) * ULLONG_MAX );
	
}
Hash_board_manager::Hash_board_manager() { 
	Hash_key_length = sizeof ( Hash_key );
}

/**
 * Here #hash_board is filled with random hash keys. */
void Hash_board_manager::init_hash_board() {
	position_hash = 0; // [C] this position_hash initialization is neccessary
				
	for ( int i = 0; i < B_HEIGHT; i++ ) 
		for ( int j = 0; j < B_WIDTH; j++) 
			for ( int c = 0; c < COLORS_NUM ; c++ ) {
				hash_board[i][j][c] = get_random_Hash_key();

				for ( int k = 0; k < B_HEIGHT; k++ ) 
					for ( int l = 0; l < B_WIDTH; l++) 
						for ( int m = 0; m < COLORS_NUM ; m++ ) {
							if ( ( ! ( i==k && j==l && c==m ) ) && ( hash_board[i][j][c] == hash_board[k][l][m] ) ) {
									cout << "KEYS CORRUPTION" << endl;
									exit(1);
								}
						}
			}
}

void Hash_board_manager::print_position_hash() { 
	log_m.buff << "Actual position hash is :" << position_hash << endl;
}

void Hash_board_manager::print_info() {
	log_m.buff << "Hash_board_manager info" << endl; log_m.flush_buffer_imp();
	log_m.buff  << "Length of Hash_key is " << sizeof ( Hash_key ) << " bytes " <<  endl; 
  log_m.buff << "RAND_MAX is " << RAND_MAX << endl; 
  log_m.buff << "max_Hash_key value is " << ULLONG_MAX << endl;
	log_m.buff << "Actual position hash is :" << position_hash << endl;
	log_m.flush_buffer();
} 

void Hash_board_manager::print_hash_board() { 
	for ( int i = 0; i < B_HEIGHT; i++ ) {
		for ( int j = 0; j < B_WIDTH; j++) 
			for ( int c = 0; c < 2 ; c++ )
				log_m.buff << setw(20) << hash_board[i][j][c]; 
		log_m.buff << endl;
	}
	log_m.flush_buffer();
}
/**
 * \param[in] coordinates_ Coordinates of a played stone.
 * \param[in] stone_color_ Stone color ( white or black ) of the played stone.
 * Zobrist hashing ... adding / removing a stone is equal to xoring with position hash	this function is called at two places ... 
 * function Board_manager::play_a_stone for single played stone and at function Board_manager::capture_block for each captured stone*/ 

void Hash_board_manager::xor_with_position_hash( Coordinates coordinates_, Stone_color stone_color_) { 
	position_hash ^= hash_board[coordinates_.get_co_row()][coordinates_.get_co_column()][stone_color_];
	/*TESTING cout << "new position hash is: " << position_hash << endl; /**/
}

															 
ostream & operator<< ( ostream & out, Stone_color A )
 { 
		if ( A == sc_black ) 
			out << "B";
		else
			out << "W";
		return out;
	}

bool operator< ( const Playable_data & A , const Playable_data & B ) { 
	if ( (int) A.get_act_main_pp() == (int ) B.get_act_main_pp() ) 
		return (  A.get_act_heuristic_pp() <  B.get_act_heuristic_pp() ) ;
	else
		return ( (int ) A.get_act_main_pp() < (int )  B.get_act_main_pp() ) ;
}
	
bool operator< ( const Playable_item & A , const Playable_item & B ) { 
	return (  A.second < B.second  ) ;/*[C] result depends on playable_data*/
}

bool operator< (   Coordinates A ,   Coordinates B ) {
	return  ( A.get_co_row() * B_WIDTH + A.get_co_column() ) < ( B.get_co_row() * B_WIDTH + B.get_co_column() ) ;
	}

ostream & operator<< ( ostream & out, Coordinates A ) { 
		out << "<" << A.get_co_row() << "," << A.get_co_column() << ">";
		return out;
	}

ostream & operator<< ( ostream & out, Playing_priority A) {
	switch (A) {
	case pp_high : out << "pp_high" ; break;
	case pp_normal : out << "pp_normal" ; break;
	case pp_low : out << "pp_low" ; break;
	case pp_none : out << "pp_none" ; break;
	case pp_ko_last : out << "pp_ko_last" ; break;
	case pp_pass : out << "pp_pass" ; break;
	};
	return out;
}

ostream & operator<< ( ostream & out, Playable_data A) { 
		out << "<pp_act:" << A.get_act_main_pp() << ", pp_heur: " << A.get_act_heuristic_pp() << ">";
		return out;
	}

ostream & operator<< ( ostream & out, Playable_item A) { 
	out << A.first << " " << A.second << endl;
	return out;
}

/**
 *\param[in] sgf_coordinates_ This is a string representation of coordinates

 * This is a constructor creating instance of Coordinates class from string representing coordinates parsed from sgf.In sgf coordinates are saved like 
 * xy where xy are of values a - s, empty or tt means pass */
Coordinates::Coordinates ( string sgf_coordinates_ ) { 
	if ( sgf_coordinates_ == "" ) { // pass 
		co_row = -1 ; co_column = -1;
		return;
	}
	if ( sgf_coordinates_[0] == 't' && sgf_coordinates_[1] == 't' ) { // take pass
		co_row = -1; co_column = -1;
		return;
	}
	co_column = sgf_coordinates_[0] - 'a' ;
	co_row = sgf_coordinates_[1] - 'a' ;
}

void Block::info() {
		log_m.buff << "This is block info " << endl; log_m.flush_buffer_imp();
		log_m.buff << " # liberties: " << liberty_count << " " << "# stones: " << stone_count << endl;
		log_m.buff<<  "stone list: " << endl;
			for ( Coordinates_vector::iterator it = stone_vector.begin() ; it != stone_vector.end() ; it++ )
				log_m.buff<< "[" << (*it).get_co_row () << "," << (*it).get_co_column() << "]";				
		log_m.buff << endl << "liberties list: " << endl;
			for ( Coordinates_set::iterator it = liberty_set.begin() ; it != liberty_set.end() ; ++it)
				log_m.buff << (*it).get_co_row() << " " << (*it).get_co_column() << endl;				
		log_m.flush_buffer();
}


stringstream & Board_manager::print_playable_map( bool print_for_sgf_printer ) {
		log_stream.str(""); /*[TD] add implicit argument of stringstream*/
	if ( ! print_for_sgf_printer ) { 
		log_m.buff << "This is Board_manager::print_playable_map()" << endl; log_m.flush_buffer_imp();
	}else
		log_stream << "This is Board_manager::print_playable_map()" << endl; 
	
	for ( Playable_map ::iterator it = playable_map.begin(); it != playable_map.end(); it++ ) 
		log_stream << (*it).first << ":" << (*it).second << endl;
	if ( print_for_sgf_printer ) 
		return log_stream;
	else {
		log_m.buff << log_stream.str();
		log_m.flush_buffer();
	}
}

stringstream & Board_manager::print_pot_eyes_map( bool print_for_sgf_printer ) {
		log_stream.str(""); /*[TD] add implicit argument of stringstream*/
	if ( ! print_for_sgf_printer ) { 
		log_m.buff << "This is Board_manager::print_pot_eyes_map()" << endl; log_m.flush_buffer_imp();
	}else
		log_stream << "This is Board_manager::print_pot_eyes_map()" << endl; 
	
	for ( Pot_eyes_map ::iterator it = pot_eyes_map.begin(); it != pot_eyes_map.end(); it++ ) 
		log_stream << (*it).first << endl;
	if ( print_for_sgf_printer ) 
		return log_stream;
	else {
		log_m.buff << log_stream.str();
		log_m.flush_buffer();
	}
}

void Unplay_manager::info() {  
			log_m.buff << "This is unplay_manager info " <<endl;	log_m.flush_buffer_imp();
			log_m.buff << "Unplayed stone :" << stone_to_unplay << endl;
			log_m.buff << "Neighbour blocks info " << endl;
			log_m.flush_buffer();
			for ( int i = 0; i < FOUR ; i++ ) 
				if ( neigbour_blocks[i] != NULL  ) 
					neigbour_blocks[i]->info();  
};

/**
 *
 * This is an initialization function. It is neccessary to run it before every search in new position. It is run from Analyze_single_position() in 
 * main.cc. It clears ( and inits ) all neccessary data structures. */
void Board_manager::init() {
			next_block_id = 1; // first block will have id == 1
			unplay_node_handle = NULL; //this is important
			number_stones_de = 0;
			ko_manager.init();
			/*SWITCHED OFF BITSET 
			bitset_at.reset(); // [C] resets ( set to all 0 ) bitsets for def. and att.
			bitset_de.reset();  
			/**/
		 	for ( int i = 0 ; i < B_HEIGHT ; i++ ) 
				for ( int j = 0 ; j < B_WIDTH; j++ ) {
					board[i][j].set_coordinates ( Coordinates ( i , j ) );
					board[i][j].set_present_block_pt(NULL);
					point_board[i][j] = ps_empty; /*POINT BOARD PROBLEM used to be ps_others*/
				} ; //constructor emptys board
				
				playable_map.clear();
				pot_eyes_map.clear();
	}

void Board_manager::print_complete_unplay_info() { 
	Unplay_node * act_node = unplay_node_handle;

	while ( act_node != NULL ) {
		act_node->unplay_manager.info();
		act_node = act_node->next;
	}
		if ( act_node == NULL ) 	
			log_m.buff << "No more unplays possible " << endl; log_m.flush_buffer(); 
}

void Board_manager::print_actual_unplay_info() { 
	if ( unplay_node_handle == NULL ) {
		log_m.buff << "No unplays possible " << endl; log_m.flush_buffer(); }
	else 
		unplay_node_handle->unplay_manager.info();
}

void Board_manager::print_stones() {
	log_m.buff << "Actual board " << endl; log_m.flush_buffer_imp();
		for ( int i = 0 ; i < B_HEIGHT ; i++ ) {
			for ( int j = 0; j < B_WIDTH ; j++ ) {
				log_m.buff << setw(2);
				switch ( point_board[i][j] ) {
					case ps_uns_at:   log_m.buff << "U" ; break;
					case ps_at:			  log_m.buff << "A" ; break;
					case ps_de:			  log_m.buff << "D" ; break;
					//case ps_pot_eye:  log_m.buff << "e" ; break;
					case ps_empty :  log_m.buff << "0" ; break;
													 /*POINT BOARD PROBLEM  case ps_others :  log_m.buff << "0" ; break; /**/
				}
			} //for
				log_m.buff << endl;
				log_m.flush_buffer();	
		} //for 
		
}
void Board_manager::print_blocks_info() {
	log_m.buff << "Board_manager::print_blocks_info() " << endl; log_m.flush_buffer_imp();
	Block * act_block_;
	for ( int i = 0 ; i < B_HEIGHT ; i++ ) {
		for ( int j = 0; j < B_WIDTH ; j++ ) {
			act_block_ = (board[i][j].get_present_block_pt());	
			if ( act_block_ == NULL ) 
				log_m.buff << setw(2) << "0";  // setw() is text formating manipulator
			else
				log_m.buff << setw(2) << act_block_->stone_color ;//<< "/" << setw(5) << act_block_->liberty_count;  // setw() is text formating manipulator
		}
	log_m.buff << endl;
	} // for
	log_m.flush_buffer();
}

/*	SWITCHED OFF BITSET 
void Board_manager::print_bitsets_info() { 
	log_m.buff << "Board_manager::print_bitsets_info() " << endl; log_m.flush_buffer_imp();
	log_m.buff << "Attackers bitset:  " << bitset_at << endl;
	log_m.buff << "Defeneders bitset: " << bitset_de << endl;
	log_m.flush_buffer();
}
/**/

void Board_manager::info () { 
log_m.buff << "Board_manager info " << endl; log_m.flush_buffer_imp();

print_blocks_info();
/*SWITCHED OFF BITSET print_bitsets_info(); /**/
print_playable_map( false );
 	
	
}



/*Board manager executive functions */


/*SWITCHED OFF BITSET

void Board_manager::add_stone_to_bitset ( Coordinates coordinates_ , Stone_color stone_color_) { 
	if ( stone_color_ == sc_black ) 
		bitset_at.set ( coordinates_.transform_to_one_dimension() );
	else
		bitset_de.set ( coordinates_.transform_to_one_dimension());
}

void Board_manager::remove_stone_from_bitset ( Coordinates coordinates_ , Stone_color stone_color_) { 
	if ( stone_color_ == sc_black ) 
		bitset_at.reset ( coordinates_.transform_to_one_dimension());
	else
		bitset_de.reset ( coordinates_.transform_to_one_dimension());
}
/**/

/**
 * \param[in] temp_board_manager This class contains all information for board initialization in simplified structures.  
 * This procedure is used to transform data stored in Temp_board_manager format ( they were gained in sgf parsing in function Sgf_parser::parse_sgf ) 
 * into Board_manager representation. */

void Board_manager:: init_from_temp_board_manager ( Temp_board_manager temp_board_manager ) { 
	//first put all stones on the board
	for ( Stone_vector::iterator it = temp_board_manager.stone_vector.begin() ; it != temp_board_manager.stone_vector.end() ; it++ )
		if ( ( (*it).get_stone_color() == sc_black ) || ( (*it).get_stone_color() == sc_white ) )  // it is a stone => i ,, play it " 
			play_a_stone( (*it).get_stone_color() , (*it).get_coordinates() ); 	
		
	//then manage the marks - for safe stones and for the space i am supposed to search 
	Block * act_block_;
	
	for ( Stone_vector::iterator it = temp_board_manager.stone_vector.begin() ; it != temp_board_manager.stone_vector.end() ; it++ ) {
//		if ( ( (*it).get_sgf_mark() == sm_safe ) )  { // mark 1 is for marking the safe stones 
	  	act_block_ = (board[(*it).get_coordinates().get_co_row()][(*it).get_coordinates().get_co_column()].get_present_block_pt());	
		  if ( act_block_ != NULL && act_block_->stone_color == sc_black ) 
				 act_block_ -> set_safe ( true );
//		}
	//	else
			if ( ( (*it).get_sgf_mark() == sm_playable ) ) {
				playable_map.insert( make_pair( (*it).get_coordinates(), Playable_data() ) );
				pot_eyes_map.insert( make_pair( (*it).get_coordinates() , 0  ));
		  	if ( act_block_ != NULL ) 
					 act_block_ -> set_safe ( false );
			}
			else
				if ( ( (*it).get_sgf_mark() == sm_at_playable ) ) { /*[C] Coordinates playable only for the attacker*/ 
					/*[C] playing priority for attacker is set to low and for defender to none ( defender won't try to play here ) */
					playable_map.insert( make_pair( (*it).get_coordinates(), Playable_data( pp_low, pp_none ) ) );
				}
	}
	
	fill_point_board(); // filling point board is neccessary part of initialization
} 


/** 
 *
 * \param[in] stone_color_ Color of stone being played. 
 * \param[in] coordinates_ Coordinates on which stone is played. 
 * \return Returns true if stone was successfully played ( it was a legal move ) otherwise returns true.
 * 
 * This is probably the most important function of position presentattion, It simulates playing a stone on the board and actualizes all related 
 * data structures. When a move is not played board_manager and also all related data structures are in same state as they were before calling 
 * the function. */

bool Board_manager::play_a_stone ( Stone_color stone_color_ , Coordinates coordinates_ ) { 

	
	if ( coordinates_ == Coordinates ( -1 , -1 ) ) { // PASS MOVE 
		
		/* [C] it is neccessary to put a record into unplay manager 
		 * however it is very simplified ... for instance no information of ko_manager or 
		 * position_hash are included ... thus are not retrieved as well*/
		Unplay_node * act_node = new Unplay_node();
		act_node->next = unplay_node_handle;
		unplay_node_handle = act_node;
		unplay_node_handle->unplay_manager.stone_to_unplay = Coordinates ( -1 , -1 );  // PASS 
		unplay_node_handle->unplay_manager.ko_manager = ko_manager; // save ko_manager for unplaying	
		ko_manager.set_active ( false ); // [C] natural ... even after a pass is ko_manager deactivated
		
		return true;
	}
	
	if ( ( board[coordinates_.get_co_row()][coordinates_.get_co_column()].get_present_block_pt() ) != NULL ) { /* stone is already there */
			/*TUNING log_m.buff << "Crosspoint" << coordinates_ << "is not empty => stone cannot be played here" << endl;	log_m.flush_buffer();*/
		return false;
	} 

	if ( ( ko_manager.get_active() ) && ( coordinates_ == ko_manager.get_retake_coordinates() )  && 
			( ( Stone_color ) ( 1 - stone_color_)  == ko_manager.get_stone_color() ) ) {
			/*TUNING  log_m.buff << "Not possible to play here ... this ko was taken last turn" << endl;	log_m.flush_buffer(); /**/
			return false;
	}

		/* Creating a proper record for stones unplaying */ 
		Unplay_node * act_node = new Unplay_node(); // creating new node in unplaying dynamic list 
		act_node->next = unplay_node_handle; // adding a node to the beginning of existing dynamic list 
		unplay_node_handle = act_node;
		unplay_node_handle->unplay_manager.stone_to_unplay = coordinates_; // filling node with unplaying information 
		unplay_node_handle->unplay_manager.number_stones_de = number_stones_de;
		/*!!!!!*/
		
		/*SWITCHED OFF BITSET 
		unplay_node_handle->unplay_manager.bitset_at = get_bitset_at();	
		unplay_node_handle->unplay_manager.bitset_de = get_bitset_de();
		/**/
		
		unplay_node_handle->unplay_manager.playable_map = playable_map; 
		unplay_node_handle->unplay_manager.pot_eyes_map = pot_eyes_map;
		unplay_node_handle->unplay_manager.position_hash = hash_board_manager.get_position_hash();
		unplay_node_handle->unplay_manager.ko_manager = ko_manager; // save ko_manager for unplaying	
		ko_manager.set_active ( false ); // [C] natural ... after playing a move ko_manager is deactivated
	

		pair < Coordinates, bool > ko_pair;
		
		 ko_pair = check_takes_ko ( stone_color_ , coordinates_ ) ;
		 if ( ko_pair.second ) {
				ko_manager.set_active ( true );
				ko_manager.set_stone_color ( stone_color_ ) ;
				ko_manager.set_ko_coordinates ( coordinates_ );
				ko_manager.set_retake_coordinates ( ko_pair.first );
			}

		/* create a new block, put one stone in it and count liberties and upgrade hash key*/
		Block * act_block_ = board_manager.create_new_block ( stone_color_ ); // creates an empty block
		act_block_->add_stone ( coordinates_ );  // 
		board[coordinates_.get_co_row()][coordinates_.get_co_column()].set_present_block_pt( act_block_); // sets a pointer from a board to the block
		hash_board_manager.xor_with_position_hash ( coordinates_, stone_color_); // updating position hash key 
		/*SWITCHED OFF BITSET add_stone_to_bitset ( coordinates_, stone_color_ );  // [C] updates bitset	/**/
		
		/* TUNING PRINTS 
		log_m.buff << stone_color_ << " coordinates: " << coordinates_ << endl;
		log_m.flush_buffer(); /**/
		
		Coordinates adjacent_coordinates_ [FOUR]; // 1 of 4 adjacent coordinates
		
		problematic_capture = false; // will there be a potentieal under the stones analyzis after capture ? 
		
		Block * adjacent_block_;

		/* In this cycle liberties of played stone are counted and four former adjacent blocks are stored for unplay_manager*/
		
		for ( int i = 0; i < 4 ; i++ ) {

				unplay_node_handle->unplay_manager.captured_neigbour_blocks[i] = false ; // initialization

				if ( ( adjacent_coordinates_[i] = get_adjacent_coordinates( coordinates_ , Direction ( i ) ) )	== Coordinates ( -1 , -1 ) ) // out of board
					continue;
																																																																				 
				adjacent_block_ =  board[adjacent_coordinates_[i].get_co_row()][adjacent_coordinates_[i].get_co_column()].get_present_block_pt() ;

				if ( adjacent_block_ == NULL ) 
						act_block_->add_liberty ( adjacent_coordinates_[i] );  //add liberty for adjacent crosspoint
				
				if ( adjacent_block_ == NULL ) 
					unplay_node_handle->unplay_manager.neigbour_blocks[i] = NULL ;
				else {
				/* MEMORY SHORTAGE PROBLEM*/
					bool store_block = true;
					for ( int k = 0; k < 4 ; k++ ) { 
						if ( unplay_node_handle->unplay_manager.neigbour_blocks[k] != NULL ) 
						if  ( adjacent_block_->id == unplay_node_handle->unplay_manager.neigbour_blocks[k]->id  ) { 
						    /*[C] this block is already stored, it is neccessary to compare through 
						     * id's because pointers are different !!!*/						
						    store_block = false; 
						}
					}
						
				/**/	
					if ( store_block ) // Block is not same as any already stored block
						unplay_node_handle->unplay_manager.neigbour_blocks[i] = new Block ( (* adjacent_block_) );
				} // else 
		}
				
		/**/	
		
		
		for ( int i = 0; i < 4 ; i++ ) {
			act_block_ = board[coordinates_.get_co_row()][coordinates_.get_co_column()].get_present_block_pt( ); //this is necccessary = when not updated 
			// in procedure merge blocks pointer might get lost
																																																					
			
			if ( adjacent_coordinates_[i] == Coordinates ( -1 , -1 )  )  // out of border
				 continue;
				adjacent_block_ =  board[adjacent_coordinates_[i].get_co_row()][adjacent_coordinates_[i].get_co_column()].get_present_block_pt() ;
				
				if ( adjacent_block_ == NULL ) //it is empty
					continue; 
				if( adjacent_block_->stone_color == stone_color_) { //adj. coordinate has the same color
					adjacent_block_->remove_liberty( coordinates_ );
					merge_blocks(  act_block_  , adjacent_block_ );
				}
				else {
					// [C] it is enemy stone -> remove liberty, test if not captured 
					adjacent_block_->remove_liberty( coordinates_ );
					
					if ( adjacent_block_->is_captured() )	{
						

						capture_block( adjacent_block_ );
						/**/

					} //if  adjacent_block_-> is_captured()
			} //else	
		}	//for
			

		/* remove actual coordinate from the set of playable coordinates*/ 
			playable_map.erase ( coordinates_ );

		/**/					
		if ( stone_color_ == sc_white ) 
			number_stones_de++;
		
		/* [TD] (dirty) - is point status consistent ? */
		/* [C] put new stone into point_board on adequate position*/
		act_block_ = board[coordinates_.get_co_row()][coordinates_.get_co_column()].get_present_block_pt(); //[C] get block of the new stone
		
		//if ( problematic_capture && act_block_ -> stone_color == sc_black && act_block_ -> liberty_count >= 4 ) 
			//act_block_ -> safe = true;
				
		if ( stone_color_ == sc_black ){ //act_block_ != NULL ) {
			if ( act_block_ -> safe ) {
				/*[C] it is neccessary to go through all stones of block and write to point board ( for case of connecting unsafe block of stones )
				 * TODO ... put this ( setting a point board somewhere else ... into merge blocks ? ) */
					for ( Coordinates_vector::iterator it = act_block_->stone_vector.begin(); it < act_block_->stone_vector.end(); it++ ) 
							point_board[(*it).get_co_row()][(*it).get_co_column()] = ps_at;
					// it is NOT NECCESSARY to delete ( now safe ) stones from potential eyes list, they will be deleted during next eye_manager analyzis
			}
			else
				point_board[coordinates_.get_co_row()][coordinates_.get_co_column()] = ps_uns_at;
			
		}
		else {
			if ( stone_color_ == sc_white ) 
	 			point_board[coordinates_.get_co_row()][coordinates_.get_co_column()] = ps_de;
			/**/
		}	
		
		/*When potential under the stones ( actual move captured more than 3 defender's stones ), his stones are erased from pot_eyes map).
		 ( only when playing under the stones is not supported*/
		if ( act_block_->safe == false && act_block_->stone_color == sc_black && problematic_capture ) {
			if ( ! params_manager.get_under_the_stones() || act_block_->stone_count == 1 )  
					// deleting "stones that cannot be captured" - only when not playing under the stones
				for ( Coordinates_vector::iterator it = act_block_->stone_vector.begin() ; it < act_block_->stone_vector.end() ; it++ )  
					pot_eyes_map.erase  ( (*it) );  
		}
		
		// [C] when it is suicide , algorithm unplays last move = this is effective and elegant way how to handle such a problem 
		if ( act_block_->is_captured() ) { 
			/*[TU]  log_m.buff << coordinates_ << " makes suicide == forbiden" << endl; 	log_m.flush_buffer();/**/
			unplay_last_stone();
			return false;  /* [C] stone was not played */
		}
		
		return true; // [C] stone was legally played
}



Coordinates get_adjacent_coordinates ( Coordinates coordinates_ , Direction direction_ ){
return ref_board.board[coordinates_.get_co_row()][coordinates_.get_co_column()].get_direct_n(direction_);
}

Coordinates get_diagonal_coordinates ( Coordinates coordinates_ , Direction direction_ ){
return ref_board.board[coordinates_.get_co_row()][coordinates_.get_co_column()].get_diagonal_n(direction_);
}

/**
 * Ref_board constructor serves for filling up a reference board.*/
Ref_board::Ref_board() {

short add_i_dir [FOUR] = { -1 , 0 , 1 , 0 };
short add_j_dir [FOUR] = {  0 , 1 , 0 , -1 };
short add_i_diag [FOUR] = { -1 , -1 , 1 , 1 };
short add_j_diag [FOUR] = { -1 ,  1 , 1 , -1 };
	
	
	for ( int i = 0 ; i < B_HEIGHT; i++ ) 
		for ( int j = 0; j < B_WIDTH; j++ ) {
			
			for ( int k = 0; k < FOUR ; k++ )  {
				if ( ( i + add_i_dir[k] < 0 ) || ( i + add_i_dir[k] >= B_HEIGHT ) || ( j + add_j_dir[k] < 0 ) || ( j + add_j_dir[k] >= B_WIDTH ) )
						board[i][j].set_direct_n( Coordinates ( -1, -1 ) ,k );
				else
					 board[i][j].set_direct_n(  Coordinates ( i+add_i_dir[k], j+add_j_dir[k] ),k );
			
				if ( ( i + add_i_diag[k] < 0 ) || ( i + add_i_diag[k] >= B_HEIGHT ) || ( j + add_j_diag[k] < 0 ) || ( j + add_j_diag[k] >= B_WIDTH ) )
						board[i][j].set_diagonal_n( Coordinates ( -1, -1 ) ,k );
				else 
					 board[i][j].set_diagonal_n(  Coordinates ( i+add_i_diag[k], j+add_j_diag[k] ),k );
			}
		}
	
}


/**
 * \param[in] block1_pt_ Pointer to a first block to merge.
 * \param[in] block2_pt_ Pointer to a second block to merge.
 * 
 * Very important function taking care of merging two blocks of stones ( of same color ) given as a pointers. Their stones and liberties 
 * are merged and one of them ( the smaller one ) is destroyed.*/

void Board_manager::merge_blocks( Block * block1_pt_ , Block * block2_pt_ ) { 
	
	if ( block1_pt_ == block2_pt_ )  {
		return; 	/*no merge , same blocks */
	}
	
	Block * destroy_block_ = block1_pt_ ;  
	Block * keep_block_ = block2_pt_;  

	if (! ( (*(block1_pt_)).stone_count < (*(block2_pt_)).stone_count ) ) {
			destroy_block_ =  block2_pt_;  
			keep_block_ = block1_pt_;  
	}  /* Now i am destroying ,,smaller" block */
	
	if ( keep_block_->safe || destroy_block_->safe ) 
		keep_block_->safe = true;

	/* TESTING PRINTS
	log_m.buff << "This is merge blocks function ... printing merged blocks info " << endl; log_m.flush_buffer();
	keep_block_->info();
	destroy_block_->info();
	*/
	
	while ( ! ( ( destroy_block_->liberty_set).empty() )  )  { 
			keep_block_->add_liberty ( * ( destroy_block_->liberty_set.begin() ) );
	 		destroy_block_->liberty_set.erase( destroy_block_->liberty_set.begin() )  ;
	}
	
	for ( Coordinates_vector::iterator it = destroy_block_->stone_vector.begin() ; it != destroy_block_->stone_vector.end() ; it++ )  { 
			/* presune vsechny kameny z jednoho bloku na druhy */
			keep_block_->add_stone ( * ( it ) );
			board[(*it).get_co_row()][(*it).get_co_column()].set_present_block_pt( keep_block_);  // !!!!
	}
			delete ( destroy_block_ ); // delete block to destroy
};

/**
 * \param[in[ stone_color_ Color of stones for a new block.
 * \return Returns pointer to newly created block.
 * 
 * Function that creates an instance of class Block ( with no stones and liberties )*/

Block * Board_manager::create_new_block ( Stone_color stone_color_ ) { 
	Block * new_block_pt_ = new Block ( stone_color_ , next_block_id++);
  return ( new_block_pt_);
}

/**
 * \param[in] block_ Pointer to a block that is captured.
 *
 * This function handles all the stuff around capturing the block. It updates actual position hash key ( by xoring out captured stones ), add liberties to 
 * adjacent blocks , updates Board_manager::playable_map and Board_manager::pot_eyes_map and safely removes the block ( by calling 
 * Board_manager::remove_block()).
 * */
void Board_manager::capture_block( Block * block_ ) {
	
Coordinates adjacent_coordinates_;	

bool possible_under_the_stones = false; /*[C] Possible under the stones teqnique ... playing under more then at least 3 captured white stones */

if ( block_->stone_color == sc_white && block_->stone_count >= 3 ) {
	possible_under_the_stones = true;
	problematic_capture = true;
}

 	for ( int i = 0 ; i < FOUR ; i++ )
		if ( unplay_node_handle->unplay_manager.neigbour_blocks[i] != NULL ) 
			if (unplay_node_handle->unplay_manager.neigbour_blocks[i]->id == block_->id) 
				unplay_node_handle->unplay_manager.captured_neigbour_blocks[i] = true; 
				
		
	for ( Coordinates_vector::iterator it = block_->stone_vector.begin() ; it < block_->stone_vector.end() ; it++ )  {
		
		hash_board_manager.xor_with_position_hash ( (*it), block_->stone_color);
		/*SWITCHED OFF BITSET  remove_stone_from_bitset ( (*it), block_->stone_color );  // [C] updates bitset /**/
		

		if ( ( ! possible_under_the_stones ) || ( params_manager.get_under_the_stones() ) ) { 
			playable_map.insert ( make_pair ( (*it) , Playable_data() ) );  // now i can play there 
//	     pot_eyes_map.insert ( make_pair ( (*it) , 0 ) );  
  	}
		if ( ! possible_under_the_stones )  // TODO is it consistent ? 
	  pot_eyes_map.insert ( make_pair ( (*it) , 0 ) );  

		
		if ( block_->stone_color == sc_white ) /*[C] decrease number of defenders stones*/
			number_stones_de--; 
		
		for ( int i = 0; i < 4 ; i++ ) {  // now i add liberties to adjacent blocks 
				if ( ( adjacent_coordinates_ = get_adjacent_coordinates( (*it) , Direction ( i ) ) )	== Coordinates ( -1 , -1 ) ) // out of board
					continue;																													 
				( board[adjacent_coordinates_.get_co_row()][adjacent_coordinates_.get_co_column()].get_present_block_pt() ) -> add_liberty ( (*it) );
		}  //for
		
	}  //for 
	/**/	
	
	remove_block ( block_);
}


/**
 * \param[in] block_ Pointer to a block to be removed.
 *
 * Function removing the block of stones itself. It empties all stones and liberties and then destroys the block.
 * */
void Board_manager::remove_block( Block * block_ ) {
	
if ( block_ == NULL ) // control 
	return; 

	for ( Coordinates_vector::iterator it = block_->stone_vector.begin() ; it < block_->stone_vector.end() ; it++ )  {	
		point_board[(*it).get_co_row()][(*it).get_co_column()] = ps_empty; /*POINT BOARD PROBLEM used to be ps_pot_eye */ // removed stones == empty points
		board[(*it).get_co_row()][(*it).get_co_column()].set_present_block_pt( NULL );
		/*[TD] is it consistent ?  */

		/**/
	}

// [TU] cout << "REMOVING BLOCK" <<endl;

		delete ( block_ );
//	block_ = NULL;
}


void Board_manager::print_to_sgf ( ostream & ostream_ ) { 
	
	ostream_ << "( ; FF[4]" << endl;
	ostream_ << "AB"; // at first set all black stones ! CHANGE INTO CONSTANTS
	for ( int i = 0 ; i < B_HEIGHT ; i++ ) 
		for ( int j = 0 ; j < B_WIDTH ; j++ ) 
			if ( board[i][j].get_present_block_pt() != NULL ) 
				if ( board[i][j].get_present_block_pt()->stone_color == sc_black ) // print
					ostream_ << "[" <<  ( char ) ( 'a' + i ) << ( char ) ( 'a' + j ) <<  "]";
	ostream_ << endl << "AW";  // now all white
	for ( int i = 0 ; i < B_HEIGHT ; i++ ) 
		for ( int j = 0 ; j < B_WIDTH ; j++ ) 
			if ( board[i][j].get_present_block_pt() != NULL ) 
				if ( board[i][j].get_present_block_pt()->stone_color == sc_white ) // print
					ostream_ << "[" <<  ( char ) ( 'a' + i ) << ( char ) ( 'a' + j ) <<  "]";
					
			
	ostream_ << ")";
}



/**
 * Initializing function, that is called in Board_manager::init_from_temp_board_manager(). It fills point board according to initial board position.
 * */

void Board_manager::fill_point_board() { 
Block * act_pt;
	
	for ( int i = 0; i < B_HEIGHT ; i++ )
		for ( int j = 0; j < B_WIDTH ; j++ ) {
			act_pt = board[i][j].get_present_block_pt();
			if ( act_pt == NULL ) { 
				point_board[i][j] = ps_empty; /*POINT BOARD PROBLEM ps_others*/
				continue;
			}
			if ( act_pt->stone_color == sc_black ) {
				if  ( act_pt-> safe ) 
		    	point_board[i][j] = ps_at;
				else
					point_board[i][j] = ps_uns_at;
				continue;
			}
			if ( act_pt->stone_color == sc_white ) {
	    	point_board[i][j] = ps_de;
				continue;
			}
			
	} // for 
	/*POINT BOARD PROBLEM
		for ( Playable_map::iterator it = playable_map.begin() ; it != playable_map.end(); it++ ) 
			if ( point_board[(*it).first.get_co_row()][(*it).first.get_co_column()] == ps_others ) // there is not a stone == a bit DIRTY
			point_board[(*it).first.get_co_row()][(*it).first.get_co_column()] = ps_empty;
		/**/
		
} // function 

/**
void Board_manager::check_under_the_stones_moves() {
Block * act_block_;
	for ( int i = 0 ; i < B_HEIGHT; i++ ) 
		for ( int j = 0 ; j < B_WIDTH; j++ ) {
			act_block_ = board[i][j].get_present_block_pt();
			if ( (( act_block_ != NULL ) && ( ! act_block_->safe ) && ( act_block_->stone_color == sc_black ) && ( act_block_->liberty_count <= 2 )) || 
					(act_block_ != NULL ) && (act_block_->stone_color == sc_white ) && ( act_block_->liberty_count <= 1 )) 
				for ( Coordinates_set::iterator it = act_block_->liberty_set.begin() ; it != act_block_->liberty_set.end(); it++ ) 
					 playable_map.insert ( make_pair ( (*it) , Playable_data ( pp_normal ))) ;
			 
		}
			
};
*/


/**
 * \param[in] stone_color_ Color of potential ko taking move.
 * \param[in] coordinates_ Coordinates of potential ko taking move.
 * \return Return's a pair < Something ( usually Coordinates ( -1 , -1 ) ) , false> if move doesn't take ko , and <Retake coordinates , true > if move does take ko 
 *
 * Important function to recognize whether a move takes ko. It is used in T_searcher::search for ordering ko_taking moves to the end of searched moves.
 * */

pair < Coordinates , bool > Board_manager::check_takes_ko ( Stone_color stone_color_ ,Coordinates coordinates_ ) { 
Coordinates adjacent_coordinates_;
Block * adjacent_block_;
Coordinates retake_coordinates_= Coordinates ( -1, -1);

if ( Coordinates ( -1, -1 ) == coordinates_ || ( ko_manager.get_active() && coordinates_ == ko_manager.get_retake_coordinates() ) ) /*[C] pass */
	return make_pair ( retake_coordinates_ , false );

int take_neigbours = 1;  /*Number of neigbours ( with one stone ) the player with it's move captures*/
int not_take_neigbours = 3;  /*Number of neigbours ( or out of board coordinates ) of opposite color player doesn't capture with his move*/
	for ( int i = 0; i < FOUR ; i++ ) {
		adjacent_coordinates_ = get_adjacent_coordinates ( coordinates_ , Direction ( i ) );
		if ( Coordinates ( -1, -1 ) == adjacent_coordinates_ ) {// out of board
			not_take_neigbours--; // out of board coordinate is equvivalent to stones that recapture ko
			continue;
		}
			
		adjacent_block_ = board[adjacent_coordinates_.get_co_row()][adjacent_coordinates_.get_co_column()].get_present_block_pt();
		if ( adjacent_block_ == NULL ) 
				return make_pair ( retake_coordinates_ , false );
		if ( adjacent_block_-> stone_color != stone_color_ && adjacent_block_->liberty_count == 1 && adjacent_block_->stone_count == 1) { 
			take_neigbours--;  // stone that will be captured in ko 
			retake_coordinates_ = adjacent_coordinates_;
		}else
		if ( adjacent_block_-> stone_color != stone_color_ && adjacent_block_->liberty_count > 1 ) {
			not_take_neigbours--; // stones neccessary to recapture ko 
		}else
				return make_pair ( retake_coordinates_ , false );
	} // for

		if ( take_neigbours == 0 && not_take_neigbours == 0 ) 
				return make_pair ( retake_coordinates_ , true );
		else 
				return make_pair ( retake_coordinates_ , false );
			
		
}


/**\brief Method to check whether two coordinates are adjacent.
 *\param[in] coordinates_1 First coordinates.
 *\param[in] coordinates_2 Second coordnates.
 *\return Returns true when given coordinates are adjacent.
 *
 * This function is useful for heuristic in statical eye analyzis ( two empty adjacent points don't create an eye ).
 */
 
bool Board_manager::check_coordinates_adjacency ( Coordinates coordinates_1 , Coordinates coordinates_2 ) { 
	if ( coordinates_1 == Coordinates ( -1, -1 ) || coordinates_2 == Coordinates ( -1,-1 ) ) 
		return false; // invalid coordinates
	if ( coordinates_1.get_co_row() == coordinates_2.get_co_row() && ( abs ( coordinates_1.get_co_column() - coordinates_2.get_co_column() ) == 1 ) ) 
				return true;
	
	if ( coordinates_1.get_co_column() == coordinates_2.get_co_column() && ( abs ( coordinates_1.get_co_row() - coordinates_2.get_co_row() ) == 1 ) ) 
				return true;
	return false;
}

/**\brief Multifunctional method to check whether stone removes liberty from  stones of oposite color, captures or ataris
 *\param[in] stone_color_ Color of played stone 
 *\param[in] coordinates_ Coordinates of played stone
 *\param[in] libertys_count_ Number of libertys of opposite block ( 1 == check capture , 2 == check atari , 0 == check take away liberty )  
 *\param[out] out_block_pt If function returns true then this is pointer to block whose liberty might be taken. 
 *\param[out] stones_num Number of stones to which liberty was taken
 *\return Returns true when a move takes away liberty of some unsafe attacker's block.
 
 * This function supports several simple heuristic : a move taking away unsafe attacker's liberty is a good one ( liberty_num_ set to 0 ) 
 * ,capture is a good one ( liberty_num_ set to 1 ) , atari is a good one ( liberty_num_ set to 2 )
 * */

bool Board_manager::check_take_liberty ( Stone_color stone_color_ , Coordinates coordinates_ , int liberty_num_ , Block ** out_block_pt ) { 
Coordinates adjacent_coordinates_;
Block * adjacent_block_;
	if ( Coordinates ( -1, -1 ) == coordinates_ ) 
		return false;

	for ( int i = 0; i < FOUR ; i++ ) {
		adjacent_coordinates_ = get_adjacent_coordinates ( coordinates_ , Direction ( i ) ); //neighbouring coordinates
		if ( Coordinates ( -1, -1 ) == adjacent_coordinates_ ) // out of board 
			continue;
		adjacent_block_ = board[adjacent_coordinates_.get_co_row()][adjacent_coordinates_.get_co_column()].get_present_block_pt(); // neighbouring block
		if ( adjacent_block_ != NULL && adjacent_block_-> stone_color != stone_color_ && ! adjacent_block_->safe && 
				( adjacent_block_->liberty_count == liberty_num_ || liberty_num_ == 0 ) )  {
			if ( out_block_pt != NULL ) 
				( * out_block_pt ) = adjacent_block_;
			return true;
		}
	} // for
	return false;
}

/**\brief Method checking whether stone removes liberty from  attacker's safe stones
 *\param[in] stone_color_ Color of played stone 
 *\param[in] coordinates_ Coordinates of played stone
 *\return Returns true when a move takes away liberty of safe attacker's block.
 
 * This function supports simple heuristic : a move taking away safe attacker's liberty is a bad one.
 * */

bool Board_manager::check_take_liberty_of_safe_attacker ( Stone_color stone_color_ , Coordinates coordinates_  ){ 
Coordinates adjacent_coordinates_;
Block * adjacent_block_;
	if ( stone_color_ != sc_white  ||  Coordinates ( -1, -1 ) == coordinates_ ) 
		return false;

	for ( int i = 0; i < FOUR ; i++ ) {
		adjacent_coordinates_ = get_adjacent_coordinates ( coordinates_ , Direction ( i ) );
		if ( Coordinates ( -1, -1 ) == adjacent_coordinates_ ) // out of board
			continue;
		adjacent_block_ = board[adjacent_coordinates_.get_co_row()][adjacent_coordinates_.get_co_column()].get_present_block_pt();
		if ( adjacent_block_ != NULL && adjacent_block_-> stone_color == sc_black && adjacent_block_->safe  )  
			return true;
	} // for
	return false;
}

/**
 * \param[in] coordinates_ Coordinates of stone representing the block I need to check ( if it is in atari or not ).
 * \param[in] stone_color_ Color of stones I need to check.
 * \param[out] stones_in_atari_num_ Returns how many stones are in self atari  
 * return Returns true if played stone is a self atari. 
 * 
 * Checks whether stones at given position are in atari. This function is used as a heuristic function.*/

bool Board_manager::check_self_atari (  Stone_color stone_color_ , Coordinates coordinates_, int * stones_in_atari_num_) { 
Coordinates adjacent_coordinates_;
Block * adjacent_block_;
bool snapback_ = false; // potential snapback
int stones = 1;
int libs = 0;
int id[4];
id[0] = -1; id[1] = -1; id[2]= -1;
int act_id;
Coordinates liberty_ = Coordinates(-1,-1); // neccessary initialization

	if ( Coordinates ( -1, -1 ) == coordinates_ ) 
		return false;

	// it is neccessary to use two cycles - one to find liberties of played stone and another to add liberties of neighbouring blocks

	for ( int i = 0; i < FOUR ; i++ ) {
		adjacent_coordinates_ = get_adjacent_coordinates ( coordinates_ , Direction ( i ) );
		if ( Coordinates ( -1, -1 ) == adjacent_coordinates_ ) // out of board
			continue;
		adjacent_block_ = board[adjacent_coordinates_.get_co_row()][adjacent_coordinates_.get_co_column()].get_present_block_pt();
		if ( adjacent_block_ == NULL ) {// liberty
			libs++;
			liberty_ = adjacent_coordinates_; // it is enough to store one liberty because when another liberty appears move is not a self atari anymore !
			if ( libs>= 2 ) 
				return false;
		}
	}
	
	for ( int i = 0; i < FOUR ; i++ ) {
		adjacent_coordinates_ = get_adjacent_coordinates ( coordinates_ , Direction ( i ) );
		if ( Coordinates ( -1, -1 ) == adjacent_coordinates_ ) // out of board
			continue;
		adjacent_block_ = board[adjacent_coordinates_.get_co_row()][adjacent_coordinates_.get_co_column()].get_present_block_pt();
		if ( adjacent_block_ == NULL ) // this case is already examined
			continue;
		if ( adjacent_block_-> stone_color == stone_color_ ) {
			act_id = adjacent_block_->id;
			if ( act_id == id[0] || act_id == id[1] || act_id == id[2] ) //this neighbour was already investigated
				continue;
			id[i]=act_id;

			// liberty that was already counted to the stone is not counted again
			if ( adjacent_block_->liberty_set.find ( liberty_ ) != adjacent_block_->liberty_set.end() ) 
				libs+=( adjacent_block_->liberty_count - 2 ); // -2 = -1 for played stone, -1 for liberty counted to the played stone
			else
				libs+=( adjacent_block_->liberty_count - 1 ); // adding liberties from the neighbour ( -1 is for played stone ) 

			if ( libs == 1 && adjacent_block_->liberty_count == 2 ) { 
				Coordinates_set::iterator it = adjacent_block_->liberty_set.begin();
				if ( coordinates_ == ( *it) ) // it is liberty where the new stone was played 
					it++;
				liberty_ = (*it);
				
			}
			
			stones+= ( adjacent_block_->stone_count ); 
		}
		else {// opponent's block
			if ( ( ! adjacent_block_-> safe ) && ( adjacent_block_-> liberty_count <= 1 ) ) // capturing enemy block 
				if ( ! snapback_ && adjacent_block_->stone_count == 1 && libs == 0) { // potential snapback
					libs = 1; // 1 liberty for ,,captured" stone but it is not saved into liberty !!! 
					snapback_ = true;
				}
				else
					return false; // not self atari
	 }	
		if ( libs >= 2 ) 
			return false;
	} // for

	(*stones_in_atari_num_) = stones;
	return true;
}


/**
 *\param[in] stone_color_ Color of played stone 
 *\param[in] coordinates_ Coordinates of played stone
 *\param[out] connects_ Tells whether capturing leads to connecting unsafe stones 
 *\param[out] stones_num Number of stones which were captured ( or NULL if not wanted )
 *\return Returns true when a move captures some opponent block.
 *
 * Heuristic function checking whether played stone captures some block is implemented by universal function Board_manager::check_take_liberty(). */

bool Board_manager::check_capture ( Stone_color stone_color_, Coordinates coordinates_ , bool * connects_ , int * stones_num_pt ) {
	Block * block_to_capture= NULL;
	bool check_ = check_take_liberty ( stone_color_ , coordinates_, 1 , &block_to_capture);
	if ( check_ && block_to_capture != NULL && connects_ != NULL ) {
		if ( stones_num_pt != NULL ) 
			(*stones_num_pt ) = block_to_capture->stone_count;
		if (  check_unsafe_neighbour ( block_to_capture ) )
				( * connects_ ) = true;
	}
	return ( check_ );
}

/**
 *\param[in] stone_color_ Color of played stone 
 *\param[in] coordinates_ Coordinates of played stone
 *\return Returns true when a move ataris some opponent block.
 *
 * Heuristic function checking whether played stone ataris some block is implemented by universal function Board_manager::check_take_liberty(). */

bool Board_manager::check_atari ( Stone_color stone_color_, Coordinates coordinates_) {
	return check_take_liberty ( stone_color_ , coordinates_, 2 );
}

/**
 *\param[in] act_block_ Pointer to block whose neighbours will be checked
 *\Return Returns true when actual block has an unsafe neighbour of opposite color.
 *
 * Supporting function for other heuristic functions.
 */

bool Board_manager::check_unsafe_neighbour ( Block * act_block_) { 
Coordinates adjacent_coordinates_;
Block * adjacent_block_;
bool safe_neighbour = false; // there is safe neighbour 
bool unsafe_neighbour = false; // there is unsafe neighbour 

	for ( Coordinates_vector::iterator it = act_block_->stone_vector.begin() ; it < act_block_ ->stone_vector.end() ; it++ ) {
		for ( int i = 0; i < FOUR ; i++ ) {
			adjacent_coordinates_ = get_adjacent_coordinates ( (*it) , Direction ( i ) );
			if ( Coordinates ( -1, -1 ) == adjacent_coordinates_ ) // out of board
				continue;
			adjacent_block_ = board[adjacent_coordinates_.get_co_row()][adjacent_coordinates_.get_co_column()].get_present_block_pt();
			if ( adjacent_block_ != NULL ) { // not empty
				if ( adjacent_block_-> stone_color != act_block_->stone_color ) { 
					if ( adjacent_block_-> safe ) 
						safe_neighbour = true;
					else 
						unsafe_neighbour = true; 
					if ( safe_neighbour && unsafe_neighbour )
						return true;
				}
			}
		}
	}
	return false;
}
/**
 * \param[in] coordinates_ Coordinates of stone which creates potential connection.
 * \param[in] stone_color_ Color of stones I need to check.
 * \return Returns true if played stone is connecting two blocks ( one of them is safe ). 
 *
 * This function supports simple heuristic: move connecting unsafe stones to safe is a very good one.
 * */
bool Board_manager::check_save_connection ( Stone_color stone_color_ , Coordinates coordinates_ ) { 
	Coordinates adjacent_coordinates_;
Block * adjacent_block_;
bool safe_connected = false;
bool unsafe_connected = false;
	
	if ( Coordinates ( -1, -1 ) == coordinates_ ) //pass
		return false;
	for ( int i = 0; i < FOUR ; i++ ) {
		adjacent_coordinates_ = get_adjacent_coordinates ( coordinates_ , Direction ( i ) );
		if ( Coordinates ( -1, -1 ) == adjacent_coordinates_ ) // out of board
			continue;
		adjacent_block_ = board[adjacent_coordinates_.get_co_row()][adjacent_coordinates_.get_co_column()].get_present_block_pt();
		if ( adjacent_block_ != NULL && adjacent_block_-> stone_color == stone_color_) 
			if ( adjacent_block_->safe  )  
				safe_connected = true;
			else
				unsafe_connected = true;
	}
	
	if ( safe_connected && unsafe_connected ) 
		return true;
	else
		return false;
}


/**
 * \param[in] coordinates_ Coordinates of stone which creates potential connection.
 * \param[in] stone_color_ Color of stones I need to check.
 * \param[in] large_blocks_ Checks whether larger blocks are connecting this is for under the stones purposes.
 * \return Returns true if played stone is connecting two ( larger ) blocks . 
 *
 * This function supports simple heuristic: connecting move is a good one.
 * 
 * */
bool Board_manager::check_connection ( Stone_color stone_color_ , Coordinates coordinates_ , bool large_blocks_ ) { 
	Coordinates adjacent_coordinates_;
Block * adjacent_block_;
int first_block_id = -1;
	
	if ( Coordinates ( -1, -1 ) == coordinates_ ) //pass
		return false;
	for ( int i = 0; i < FOUR ; i++ ) {
		adjacent_coordinates_ = get_adjacent_coordinates ( coordinates_ , Direction ( i ) );
		if ( Coordinates ( -1, -1 ) == adjacent_coordinates_ ) // out of board
			continue;
		adjacent_block_ = board[adjacent_coordinates_.get_co_row()][adjacent_coordinates_.get_co_column()].get_present_block_pt();
		if ( adjacent_block_ != NULL && adjacent_block_-> stone_color == stone_color_ && ( ! large_blocks_ || adjacent_block_-> stone_count >= 2 ) ) 
			if ( first_block_id == -1 )
				first_block_id = adjacent_block_->id;
			else
				if ( first_block_id != adjacent_block_->id ) 
					return true;
	}
	return false;
}
	
/**/

/* \brief Check's eye connectivity
 * \param[in] Coordinates_vector vector of full eyes
 * \return Returns true when at least two eyes in the input vector are connected, otherwise returns false 
 *
 * Chceks whether are any two ( full ) eyes in given Coordinates vector connected by defender's stones ( == group is alive ),
 * this function is called after at least two full eyes are found in the group. Into internal array  \c neighbours saves ( different ) pointers to the 
 * neigbouring blocks of an eye ... when there is already same pointer for a different eye it returns true*/ 

bool Board_manager::check_eyes_connection ( Coordinates_set & eyes_set ) { 
pair < Block * , Coordinates > neighbours[4] ; // array of neigbhours of an eye, block * is a neighbour , Coordinates is to check whether neighbour was added by the same od different eye

Block * adjacent_block_;
Coordinates adjacent_coordinates_;
  // init 
	for ( int i = 0; i < 4; i++ ) 
		neighbours[i].first = NULL;

	for ( Coordinates_set::iterator it = eyes_set.begin(); it != eyes_set.end() ; ++it ) 
		for ( int i = 0; i < 4; i++ ) {
			adjacent_coordinates_ = get_adjacent_coordinates ( (*it) , Direction ( i ) );
			if ( Coordinates ( -1, -1 ) == adjacent_coordinates_ ) // out of board
				continue;
		  adjacent_block_ = board[adjacent_coordinates_.get_co_row()][adjacent_coordinates_.get_co_column()].get_present_block_pt();
  		if ( adjacent_block_ == NULL || adjacent_block_-> stone_color != sc_white ) 
				continue;
			
   		for ( int j = 0; j < 4; j++ ) {
				if ( neighbours[i].first == NULL ) { // save new neighbour
					neighbours[i] = make_pair ( adjacent_block_, (*it) ); 
					break;
				}
				if ( neighbours[i].first != adjacent_block_ ) // different neigbour
					continue;
				if ( neighbours[i].first == adjacent_block_ ) // same neighbour 
					if ( neighbours[i].second == (*it) ) // neighbour was added from the same block 
						break;
					else	// neighbour was added from different block -> they are connected
						return true;
				
			}
		} // for ( int i ...  )
	return false;
}
	
/**
 * Very importatn function handling unplaying moves. It unplays last move ( the last one in a dynamic list of unplayed moves ) by actualizing Board_manager
 * data structures to saved values.
 * */

void Board_manager::unplay_last_stone() {  //USES unplay manager to unplay last stone

	if ( unplay_node_handle == NULL ) {
		log_m.buff << " Board_manager::unplay_last_stone() -> Unplay_node_handle points on NULL " << endl; log_m.flush_buffer();	
		return; // problem
	}

	if (unplay_node_handle->unplay_manager.stone_to_unplay == Coordinates ( -1 , -1 ) ) {// [C] unplaying pass => little changes
	
		ko_manager = unplay_node_handle -> unplay_manager.ko_manager;
		Unplay_node * pom_handle = unplay_node_handle;
		unplay_node_handle = unplay_node_handle->next;
		delete ( pom_handle );
		
		return;  
	}

	/*[C]retriving old information that might be simply copied*/
	playable_map = unplay_node_handle->unplay_manager.playable_map; 
	ko_manager = unplay_node_handle -> unplay_manager.ko_manager;
	number_stones_de = unplay_node_handle->unplay_manager.number_stones_de;			
	hash_board_manager.set_position_hash( unplay_node_handle->unplay_manager.position_hash);  
	pot_eyes_map = unplay_node_handle -> unplay_manager.pot_eyes_map;
	/*SWITCHED OFF BITSET 
	bitset_at = unplay_node_handle -> unplay_manager.bitset_at;
	bitset_de = unplay_node_handle -> unplay_manager.bitset_de;/**/
	/**/


	Block * act_block_ = board[unplay_node_handle->unplay_manager.stone_to_unplay.get_co_row()]
														[unplay_node_handle->unplay_manager.stone_to_unplay.get_co_column()].get_present_block_pt();
	
	Stone_color unplay_stone_color_ = act_block_->stone_color; /*color of unplayed stone*/
	
	remove_block ( act_block_ ) ; // control on NULL is included in procedure removes block
	
  Coordinates	adjacent_coordinates_;

	for ( int i =0 ; i < FOUR ; i++ ) {
		 adjacent_coordinates_ = get_adjacent_coordinates ( unplay_node_handle->unplay_manager.stone_to_unplay , Direction (i) );//get adjacent coordinates
		 if ( adjacent_coordinates_ == Coordinates ( -1, -1 ) ) // out of board;
			 continue;
			remove_block ( board[adjacent_coordinates_.get_co_row()][adjacent_coordinates_.get_co_column()].get_present_block_pt() ); 
		}

	// NOW all neigbours are removed ... it is time to set new neigbours
  Block * pom_block_;
	
	for ( int i =0 ; i < FOUR ; i++ ) {
	act_block_ =  unplay_node_handle->unplay_manager.neigbour_blocks[i];

		if ( act_block_ != NULL ) {

			for ( Coordinates_vector::iterator it = act_block_ ->stone_vector.begin() ; it < act_block_ ->stone_vector.end() ; it++ ) {
				
				pom_block_ = board[(*it).get_co_row()][(*it).get_co_column()].get_present_block_pt( ); /*now shoulde be removed == NULL */
				if ( pom_block_ != NULL ) {
							/*TUNING*/	cout << "UNPLAY MANAGER PROBLEM" << endl;
							exit ( 1 );
				}
	    	board[(*it).get_co_row()][(*it).get_co_column()].set_present_block_pt( act_block_ );

			// updating Board_manager::point_board according to actual stone ( color , safe/unsafe )
				if  ( act_block_->stone_color == sc_white ) 
					point_board[(*it).get_co_row()][(*it).get_co_column()] = ps_de;
				else {
					if ( act_block_-> safe ) 
						point_board[(*it).get_co_row()][(*it).get_co_column()] = ps_at; //safe attacker
					else
						point_board[(*it).get_co_row()][(*it).get_co_column()] = ps_uns_at; // unsafe attacker
			 }
				

			} // for stones
		} // if 
	} //for 

	Block * adjacent_block_;
	
	for ( int i =0 ; i < FOUR ; i++ ) {
		
	if ( ! unplay_node_handle->unplay_manager.captured_neigbour_blocks[i] )
		continue;

		act_block_ =  unplay_node_handle->unplay_manager.neigbour_blocks[i];
		if ( act_block_ == NULL ) 
			continue;
		
			for ( int j = 0; j < FOUR ; j++ ) {  // now i add liberties to adjacent blocks 
				
				for ( Coordinates_vector::iterator it = act_block_ ->stone_vector.begin() ; it < act_block_ ->stone_vector.end() ; it++ ) {
					
					if ( ( adjacent_coordinates_ = get_adjacent_coordinates( (*it) , Direction ( j ) ) )	== Coordinates ( -1 , -1 ) ) // out of board
						continue;																													 
					 adjacent_block_ = board[adjacent_coordinates_.get_co_row()][adjacent_coordinates_.get_co_column()].get_present_block_pt();
					 if ( adjacent_block_ == NULL ) 
						 continue;
					if ( adjacent_block_ != act_block_ )
						adjacent_block_ -> remove_liberty ( ( *it ) ) ;
				} // for
			} // for
	} // for 
	
	Unplay_node * pom_handle = unplay_node_handle;
	unplay_node_handle = unplay_node_handle->next;
	delete(	pom_handle );
	

}
