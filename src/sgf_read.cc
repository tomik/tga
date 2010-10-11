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

/**\file sgf_read.cc
 * \brief Contains implementation of methods related to sgf parsing ( \ref page_5 ).
 *
 * In this file methods performing  sgf parsing of input position are implemented. Generally there is a parsing automata managing whole process.
 */

#include "sgf_read.h"


/**/

using namespace std;


void Temp_board_manager::print() {
	for ( Stone_vector::iterator it = stone_vector.begin() ; it != stone_vector.end() ; it++ )
		log_m.buff << "souradnice:" << (*it).get_coordinates() << "	barva" << (*it).get_stone_color() << endl;				
	log_m.flush_buffer();
}

/**
 * \param[int] act_char Represents actually read character from sgf intput.
 * \param[int] act_text Contains string from last saved value
 * \return This function returns information whether was the character accepted as ,,important" one ( it transfers automata from actual state to some 
 * new one )
 *
 * This functino tries to find a transition from actual state to a new one "along the edge" represented by act_char character. Then performs actions 
 * connected with previous state ( like placing a stone , ... ) and switches automata to new state.
 * */
bool Sgf_parser::resolve(char act_char , string act_text ) {
bool found_ =false;
Parser_state new_state_;
	for ( State_function_vector::iterator it = state_function.begin() ; it != state_function.end(); it++) 
		if ( State_function_record( actual_state, act_char , actual_state ) // third parametr is unimportant - anything ! 
					== (*it) ) {
					found_ = true;               // record was found
					new_state_ = (*it).get_new_state(); // retrieve new_state
					break;
		}
	
 if ( found_ ) {
	 actual_state = new_state_;// setting up a new state 
		
	switch ( new_state_ ) {
		case aw_data  : // new state is awaiting data -> it means that actual token must be saved
			if ( ! act_text.empty() ) 
		 		actual_token = act_text; // saving actual token
		  break;
	 	case aw_next_token : // awaiting next token -> it means that old token and it's data ( act_text ) must be handled
		
		 if ( actual_token == token_adjust_b_stone ) { 
				Coordinates coordinates_ = Coordinates ( act_text );
/*			 	int index = coordinates_.get_co_row() * 8 + coordinates_.get_co_column();
					record_.set_black_stone ( index );*/
				temp_board_manager.stone_vector.push_back ( Stone(sc_black, sm_none, coordinates_));
		 }	
		 if ( actual_token == token_adjust_w_stone ) { 
					Coordinates coordinates_ = Coordinates ( act_text );
/*	 		  int index = coordinates_.get_co_row() * 8 + coordinates_.get_co_column();
					record_.set_white_stone ( index );*/
				temp_board_manager.stone_vector.push_back ( Stone(sc_white, sm_none, coordinates_));
		 }	 
		 if ( actual_token == token_comments ) {
			 		temp_board_manager.comments = act_text; // potential comments to the problem
			 //actual_state = aw_pattern_data;  
			 /* DIRTY TRICK - setting new state manually  
			 this is because the rest of file will be attached to bit_tape position representation
			 in the ,,compiled database", this state won't change until eof*/
		 }
		 if ( actual_token == token_mark_safe ) {
			 	Coordinates coordinates_ = Coordinates ( act_text );
/*			 	int index = coordinates_.get_co_row() * 8 + coordinates_.get_co_column();
					record_.set_mark( index );*/
				temp_board_manager.stone_vector.push_back ( Stone(sc_none, sm_safe, coordinates_));
		 }
		 
		 if ( actual_token == token_mark_playable ) {
			 	Coordinates coordinates_ = Coordinates ( act_text );
/*			 	int index = coordinates_.get_co_row() * 8 + coordinates_.get_co_column();
					record_.set_mark( index );*/
				temp_board_manager.stone_vector.push_back ( Stone(sc_none, sm_playable , coordinates_));
		 }
		 
		 if ( actual_token == token_mark_at_playable ) {
			 	Coordinates coordinates_ = Coordinates ( act_text );
/*			 	int index = coordinates_.get_co_row() * 8 + coordinates_.get_co_column();
					record_.set_mark( index );*/
				temp_board_manager.stone_vector.push_back ( Stone(sc_none, sm_at_playable , coordinates_));
		 }
		 
		 if ( actual_token == token_size) {
				temp_board_manager.board_size = atoi ( act_text.c_str() );
		 }
				
		 break;
		 
	 default:	break;
	 }; // switch
	 
	 
	 return true;
 } // if found_
 
 return false;
};

/**
 * In constructor, the state function and tokens are defined.*/

Sgf_parser::Sgf_parser() { for ( int i = 0; i < 4 ; i++ ) 
	
	// fill data to state function
	state_function.push_back(State_function_record(aw_beginning,'(',aw_token));
	state_function.push_back(State_function_record(aw_token,';',aw_token));
	state_function.push_back(State_function_record(aw_token,')',aw_beginning));
	state_function.push_back(State_function_record(aw_token,'[',aw_data));
	//state_function.push_back(State_function_record(aw_next_token,'(',aw_variation));
	//state_function.push_back(State_function_record(aw_next_token,'(',aw_pattern_data));
	
	state_function.push_back(State_function_record(aw_next_token,')',aw_beginning));
	state_function.push_back(State_function_record(aw_next_token,';',aw_token));
	state_function.push_back(State_function_record(aw_next_token,'[',aw_data));
	state_function.push_back(State_function_record(aw_data,']',aw_next_token));
	
	token_play_w_stone = "W";
	token_play_b_stone = "B";
	token_adjust_b_stone = "AB";
	token_adjust_w_stone = "AW";
	token_comments = "C";
	token_mark_safe = "TR";
	token_mark_playable = "SQ";
	token_mark_at_playable = "CR";
	token_size ="SZ";

// token_pattern_type = "GN";

};


/**
 * \param[in] filename_ Name of input sgf file.
 * return Returns whether sgf was sucessfully parsed.
 * 
 * This function coordinates transfer of sgf file into pos_db_record using Sgf_parser::resolve() function. If returns false, parsing was unsuccessfull
 * and no search is performed.
*/

bool Sgf_parser::parse_sgf ( string filename_ ) { 
	
char act_char;
fstream file_stream_in;
string act_text;
	
	actual_state = aw_beginning; /* important */
	
	file_stream_in.clear();
	file_stream_in.open( filename_.c_str() );
	if ( ! file_stream_in.good() ) {
		log_m.buff << "error, unable to open file: " << filename_ << endl; 
		log_m.flush_buffer();	
	}
	
/* LOGGING	log_m.buff << "Parsing of sgf file " <<  filename_ << " started."<< endl; 
						log_m.flush_buffer(); /**/
	
	while ( true)  	{
		act_char = file_stream_in.get();		
			
			if ( ! file_stream_in.good() ) /* the order is important 1. get a char , 2. test good */
				break;
		
			/* skip white characters */
			if ( actual_token != token_comments && ( act_char == ' ' || act_char == '\t' || act_char == '\n' ) ) 
				continue;
			
			if ( sgf_parser.resolve(act_char, act_text ) )  { 
					act_text.clear(); // it is significant character [ , ] , ; , ... 	
			}
			else 							
				act_text += act_char; 	// it is a usual character - part of token or data section
		}
	
//		 temp_board_manager.set_attached_data("pattern " + filename_ + " : " + act_text); /* packs data into record */

		 /*LOGGING	log_m.buff << "Parsing of sgf file "<< filename_ << " finished succesfully." << endl; 
	    					log_m.flush_buffer(); 	/**/
	
	file_stream_in.close();

	if ( temp_board_manager.board_size != B_HEIGHT ) {
		log_m.buff << "Incorrect board size found in position : " << filename_ << endl;
		log_m.flush_buffer();
		return false;
	}
	return true;
	
}
