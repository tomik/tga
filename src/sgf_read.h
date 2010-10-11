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

/**\file sgf_read.h
 * \brief Header file to sgf_read.cc
 *
 * In this header file classes and data structures neccessary for proper input sgf file parsing are declared.
 * */ 


#ifndef SGF_READ_ 
#define SGF_READ_

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include "board.h"
#include "log_manager.h"


using namespace std;

/**\brief Parser automata state enum. * This enum represents possible states that a parsing automata might be in.
 * */

enum Parser_state {  
	aw_beginning ,  //!< waiting for "("
	aw_token ,  		//!< awaits token : for instance "W" "B" ... 
	aw_next_token,  //!< one token accepted, awaits another
	aw_data,				//!< got token and symbol "[" 
	aw_pattern_data //!< awaits ,,useful" data - variations from position 
		
};

/**\brief  This class represents one record in parsing automata ( Sgf_parser ) state function ( Sgf_parser::state_function ).
 *
	 * This class simulates autamata's state function - two given "input" arguments : #old_state and #symbol results into  a #new_state. 
*/

class State_function_record {  
	Parser_state old_state; //!< This is the state parsing automata is actually in.
	char symbol; //!< Actual symbol - represents an edge along which automata goes to the new_state.
	Parser_state new_state; //!< New state of automata after the transition.
public:
	/**This operator== is neccessary for finding an appropriate record in Sgf_parser::state_function_vector.
	 */
	bool operator==( State_function_record A ) { 
		if ( A.old_state == (this)->old_state && A.symbol == (this)->symbol )
				return true;
		return false;
	}
	
	State_function_record(Parser_state old_state_,char symbol_, Parser_state new_state_) :
					old_state(old_state_), symbol(symbol_), new_state(new_state_)	{};
	Parser_state get_new_state() { return new_state; }
};

typedef std::vector<State_function_record> State_function_vector;

/**\brief This class conducts sgf input interpretation.
 *
 * This class contains functions to read sgf input and convert it into board representation. It is actually a simple finite automata. State function
 * is simulated with State_function_vector
*/

class Sgf_parser { 
	private:
		Parser_state actual_state;	//!< state of parsing automata
		State_function_vector state_function;	//!< three information ... [old_state,character,new_state]

		string actual_token; //!< last accepted token 
		string token_play_w_stone; //!< token representing played white stone
		string token_play_b_stone; //!< token representing played black stone
		string token_adjust_w_stone; //!< token representing ,,set" white stone  
		string token_adjust_b_stone; //!< token representing ,,set" black stone  
		string token_comments; //!< separates part of position and ,,useful" data 
		string token_mark_safe; //!< marks are used to specify patterns - triangle is used to mark safe stones
		string token_mark_playable;   //!< second type of mark - square is used to mark playable coordinates
		string token_mark_at_playable; //!< third type of mark - is used to mark coordinates that are playable only by attacker
		string token_size; //!< token for verifying size of the board 
//		string token_pattern_type; 
	
		
		Temp_board_manager temp_board_manager; //!< Temporal board manager for storing results of transfer sgf -> board representation.
		
	public:
		/**\brief Constructor.*/
		Sgf_parser();
		/**\brief Initialization function.*/
		void init() { actual_state = aw_beginning; 
									temp_board_manager.init();}  
		void set_actual_state(int new_state) { actual_state = Parser_state(new_state); };
		int get_actual_state() { return actual_state; };
		/**\brief Resolving single characters of sgf input.*/
			bool resolve(char , string );
			
		/**\brief Function coordinating process of sgf parsing*/
		bool parse_sgf ( string ); 
		
		void print_temp_board_manager () { temp_board_manager.print(); };
		
		Temp_board_manager & get_temp_board_manager () {return temp_board_manager; } 
};

extern Sgf_parser sgf_parser;

#endif
