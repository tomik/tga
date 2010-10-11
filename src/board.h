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

#ifndef BOARD_
#define BOARD_

/**\file board.h
 * \brief Header file for board.cc
 *
 * In this header file classes and data structures neccessary for proper moves playing and unplaying are declared.
 */

#include <string>
#include <set>
#include <map>
#include <vector> 
#include <iostream>
#include <iomanip>
#include <sstream>
#include <bitset>
#include <limits.h>
#include <stdlib.h>

#include "log_manager.h"
#include "p_manager.h"
#include "constants.h"

using namespace std;

/**\brief Stone color. 
 *
 * Stone color is used for simple distinguishing attackers and defenders stones. */
enum Stone_color { 
	sc_black, 
	sc_white,
	sc_none
};

/*\brief Marks extracted from sgf input*/
enum Sgf_mark { 
	sm_safe, //!< On coordinate there is a safe stone.
	sm_playable, //!< It is possible to play at a coordinate.
	sm_at_playable, //!< Playable only for attacker, not defender ( coordinates on the edge ).
	sm_none //!< No sgf mark.
};

/**\brief Enum of possible values in Board_manager::Point_board*/

enum Point_status { 
	ps_at,  //!<Attacker' stone ( safe ).
	ps_uns_at, //!<Unsafe attacker's stone.
	ps_de,  //!<Defender's stone.
	ps_pot_eye, //!<Potential eye.
	ps_empty, //!<Empty point.
	ps_others   
};  

ostream & operator<< ( ostream & out, Stone_color A );

/** Structure representing coordinates of one intersection.*/

/**\brief Priority of a move.*/
enum Playing_priority {  
	pp_high, //!< High priority.
	pp_normal, //!< No special priority.
	pp_low, //!< Low priority.
	pp_pass,  //!< Very low, passes are played very last but before ko.
	pp_ko_last, //!< Ultra low, ko taking moves are played last.
	pp_none //!< Not played at all.
		
};

ostream & operator<< ( ostream & out, Playing_priority A); 

/**\brief Class representing a single coordinates.
 *
 * This class represents coordinates on a board. It is used throughout the whole program. [-1,-1] are special coordinates standing for pass ( when meaning 
 * a move ) or a position outsite bounds of the board.
 * */
class Coordinates { 
	private:
	int co_row ; //!< Row coordinate.
	int co_column ; //!<Column coordinate.
	public:	
	Coordinates() {};
	Coordinates( int i , int j ) { co_row = i ; co_column = j; };
	Coordinates ( string ); //!< constructor for transfer of string coordinates to coordinates
	int get_co_row() const { return co_row; }  
  int get_co_column() const { return co_column; } 
	void set_co_row ( int co_row_ ) { co_row = co_row_;  }
	void set_co_column ( int co_column_ ) { co_column = co_column_; }
	
	bool operator== ( Coordinates A ) { 	
			if ( A.get_co_row() == (this)->co_row  && A.get_co_column() == (this)->co_column ) 
				return true;
			return false;
	}
	
	/**\brief Transforms two dimensional coordinates to one dimensional.*/
	int transform_to_one_dimension () { return ( co_row * B_WIDTH + co_column ); }
		
};

/**\brief Useful information added to playable coordinates.
 *
 * Playable_data class provides additional information about a possible point to play - especially playing priorities which are used to 
 * sort moves in order to get maximum alfa-beta pruning efficiency. Distinguishing playing priorities for attacker and defender provides algorithm with 
 * possibility to effectivily sort proposed moves according to their priorities ( for whom is to move )
 * */

class Playable_data { 
	private:
		Playing_priority main_pp_at;  //!<main priority ( for attacker ), decide whether ko move will be played last, whether some move won't be played ... it is rather ,,organizational", while heuristic priorities try to speed up search
		Playing_priority main_pp_de; //!< same as #main_pp_at for defender
		
		Playing_priority act_main_pp;   //!<actual playing priority ( only one for both colors ) when transforming from at/de playing priorities i decide according to stone color of player to move in T_search::search()
		
		int act_heuristic_pp; //!< Heuristic priorities are less important ( when sorting moves according to priorities they are second criterium), 					   but they increase search speed 
		
		
	public:
		
		/*[C] constructors */
		Playable_data() { main_pp_at = pp_normal; main_pp_de = pp_normal; act_heuristic_pp = 0; };
		Playable_data( Playing_priority main_pp_both_) 
			{ main_pp_at = main_pp_both_; main_pp_de = main_pp_both_; act_heuristic_pp = 0;};
		Playable_data( Playing_priority main_pp_at_ , Playing_priority main_pp_de_ ) 
			{main_pp_at = main_pp_at_;
		   main_pp_de = main_pp_de_;};

		Playing_priority get_main_pp_at ( ) { return main_pp_at; }
		Playing_priority get_main_pp_de ( ) { return main_pp_de; }
		Playing_priority set_main_pp_at ( Playing_priority pp_ ) { main_pp_at = pp_;}
		Playing_priority set_main_pp_de ( Playing_priority pp_ ) { main_pp_de = pp_;}
		
		Playing_priority get_act_main_pp ( ) const { return act_main_pp; }
		Playing_priority set_act_main_pp ( Playing_priority pp_ ) { act_main_pp = pp_ ;}

		int get_act_heuristic_pp ( ) const { return act_heuristic_pp; }
		void increase_act_heuristic_pp ( int diff ) { act_heuristic_pp += diff;
			/*
			if (diff < 0 && act_heuristic_pp >= diff && act_heuristic_pp <= 0 ) 
				act_heuristic_pp = diff;
			else if ( diff > 0  && act_heuristic_pp <= diff) 
					act_heuristic_pp += diff;
					*/}

		/**\brief Synchronizes temporar playing priorities to playing priorities */
		void sync_playing_priorities ( Stone_color stone_color_to_move_ ) 
			{ 
				if ( stone_color_to_move_ == sc_black ) 
					act_main_pp = main_pp_at;
				else 
				  act_main_pp = main_pp_de; 
		}	 // function
};

typedef pair < Coordinates , Playable_data > Playable_item;
ostream & operator<< ( ostream & out, Playable_data A); 
ostream & operator<< ( ostream & out, Playable_item A); 


typedef map < Coordinates, Playable_data > Playable_map;
typedef vector < Playable_item > Playable_vector;
typedef map < Coordinates, int > Pot_eyes_map; /*[C] so far second argument is unimportant, but might be in the future*/
/** Contains two arrays for representing neigbours of point on the board.*/


bool operator< ( const Playable_data & A , const Playable_data & B ) ; 
bool operator< ( const Playable_item & A , const Playable_item & B ) ;  


/**\brief Contains neighbouring coordinates.
 *
 * There are data structures in this class to save neigbouring ( both direct and diagonal ) coordinates.*/
class Neighbors { 
	private:
		Coordinates diagonal_n [FOUR];  //!< diagonal neigbours starting from the " top left " in the direction of the clocks 
		Coordinates direct_n [FOUR]; //!< direct neigbours starting from the "top left" as well
	public:
		Coordinates get_diagonal_n ( int i ) { return diagonal_n [ i ] ; };
		Coordinates get_direct_n ( int i ) { return direct_n [ i ] ; };
		void set_diagonal_n ( Coordinates diagonal_n_ , int i) {  diagonal_n[i] = diagonal_n_ ; };  // add controls of boundaries
		void set_direct_n (   Coordinates direct_n_,  int i ) {  	direct_n[i] = direct_n_ ;  };
};


/**\brief Reference board for getting adjacent coordinates.
 *
 * This class contains board where every point carries coordinates to it's neigbours ( in form of Neigbors instance ).*/
class Ref_board { 
	public: 
		Neighbors board [B_HEIGHT][B_WIDTH]; //!<This array containts coordinates of neighbouring coordinates.
		/**\brief Constructor server for filling #board*/
		Ref_board();
};

/**\brief Class used for transformation from sgf to board representation.
 *
 * This class carries information about a coordinate and potential stone placed at it. It is used when Board_manager is initialized from Temp_board_manager 
 * in Board_manager::init_from_temp_board_manager() function.
 */

class Stone {  
		private:
			Stone_color stone_color; //!< Color of stone ( might be as well sc_none for empty coordinates )
			Sgf_mark sgf_mark; //!< Added sgf mark to the coordinate ( stone ), carrying special meaning ( stone is safe, coordinate is playable, ... )
			Coordinates coordinates; //!< Coordinates of the stone.
		public: 
			Stone( Stone_color stone_color_ , Sgf_mark sgf_mark_ , Coordinates coordinates_ ) 
					 { stone_color = stone_color_; sgf_mark = sgf_mark_ ;coordinates = coordinates_;} 
      Coordinates get_coordinates() { return coordinates;} 
			Stone_color get_stone_color() { return stone_color;}
			Sgf_mark get_sgf_mark() 			{ return sgf_mark; }
	
};

typedef vector < Stone > Stone_vector;

/**\brief Temporar board manager.
 *
 * In this class are stored information gained during sgf parsing. Initial board position is then retrieved from this class throughout the function
 * board_manager::init_from_temp_board_manager().
 **/

class Temp_board_manager { 
	private: 
//		string attached_data; //!< attached data for pos_db_record
		Stone_vector stone_vector; //!< Vector of positioned stones together with their sgf marks ( even stones of none color are allowed : for marks related to empty coordinates )
		int board_size; // must be set to B_HEIGHT by parser 
		string comments; // comments on the position 
		friend class Sgf_parser; //!< Sgf parser consists Temp_board_manager -> friend
		friend class Board_manager;
	public:
		//	void set_attached_data ( string attached_data_ ) { attached_data = attached_data_;} 
		//string get_attached_data ( ) { return attached_data; } 
		
		/**\brief Testing function.*/
		void print(); 		
		void init() { //attached_data = "";
									comments = "";
									stone_vector.clear();} ;
		string get_comments() { return comments;}
};

/** Comparing operator. 
		This operator is needed since I am storing coordinates in set - requires comparing operator*/

bool operator< ( Coordinates A ,   Coordinates B ); 

ostream & operator<< ( ostream & out, Coordinates A ); 

typedef set < Coordinates > Coordinates_set;
typedef vector < Coordinates  > Coordinates_vector;

class Board_manager;  

/**\brief Block of stones.
 *
 * This is one of basic classes. Every instance represent a single block of firmly connected stones ( of one color ).
*/
class Block { 
	private:
		int id;             //!< identification number of block
		Stone_color stone_color; //!< color of the block 
		int liberty_count;  //!< number of liberties
		int stone_count;    //!< number of stones creating block
		bool safe; 					//!< true => block is safe, false => might be captured ( usually for attacker's stones ) 
		Coordinates_set liberty_set; //!< set of liberties is for quick search
		Coordinates_vector stone_vector; //!< search among stones is not neccessary -> vector 
		friend class Board_manager; 
	public:	
		Block ( Stone_color stone_color_ , int id_ )
			{ liberty_count = 0; stone_count = 0; stone_color = stone_color_; safe= false;  liberty_set.clear(); stone_vector.clear();  id = id_ ; };
		void set_id ( int id_ ) { id = id_ ;} 
		int  get_id ( ) { return id; }
		void set_safe( bool safe_ ) { safe = safe_ ;};

		/**\brief Adding stone to a block.*/
		
		void add_stone ( Coordinates coordinates_ ) { 
			stone_vector.push_back( coordinates_ ); 
			stone_count++;
		}
		
		/**\brief Adding liberty to a block.*/
		
		void add_liberty ( Coordinates coordinates_ ) { 
			if ((liberty_set.insert(coordinates_ )).second == true )	
				liberty_count++; // liberty was not there => increase # of liberties. 
		}

		/**\brief Removing liberty of a block.*/
		void remove_liberty ( Coordinates coordinates_) {
			if ( liberty_set.erase ( coordinates_ ) > 0 ) { // removed liberty -> decrease count 
				liberty_count--;
			}
		}
		
		/**\brief Check if given coordinate is a liberty.*/
		bool is_liberty ( Coordinates coordinates_ ) { return  ( liberty_set.find( coordinates_ ) != liberty_set.end() );} 
		
		/**\brief Check if block is captured.*/
		bool is_captured ( ) { return (liberty_count <= 0) ; };

		/**\brief Testing function.*/
		void info(); 

		Stone_color get_stone_color () { return stone_color; };
				
};

/**\brief Representation of 1 intersection on the board.
 *
 * Instancies of this class are used to represent go board in Board_manager::board. It contains coordinates together with pointer to a block of stones 
 * which contains stone lying on this coordinate ( NULL when it is empty ).
 * */
struct Cross_point {  
	private:
		Coordinates coordinates; // !< coordinates of cross point 
		Block * present_block_pt; // !< block containing stone on coordinates, NULL if it is empty
	public:
		Cross_point(){ present_block_pt = NULL; };
		Block * get_present_block_pt() { return present_block_pt; }
		void set_present_block_pt( Block * present_block_pt_) { 
			present_block_pt = present_block_pt_; 
		}
		Coordinates get_coordinates() { return coordinates; };
		void set_coordinates( Coordinates coordinates_ ) { coordinates = coordinates_ ; };
	};
	
/**\brief Basic directions*/
enum Direction { 
	north , east , south , west
};


typedef Block *  Block_pt ;
typedef vector < Block_pt> Block_vector;

		
Coordinates get_adjacent_coordinates ( Coordinates , Direction);
Coordinates get_diagonal_coordinates ( Coordinates , Direction);


/**\brief Class handling ko.
 *
 * This class handles appearences of ko. Algorithm is capable to store information only of one ko in every turn ( double ko's and another pecularities 
 * are handled during a tree search, see T_searcher::search */ 
class Ko_manager { 
	private:
		Coordinates ko_coordinates;  //!< coordinates of stone that took the ko
		Coordinates retake_coordinates; //!< where must play opponent to retake the ko,when ko_manager is not active, retake coordinates are Coordinates(-1,-1), this is demanaded by t_search
		Stone_color stone_color;   //!< color of stone that took the ko 
		bool active;  //!< if true then some ko was taken in actual turn

	public: 
		void init ( ) { active = false; retake_coordinates = Coordinates ( -1 , -1 ); } 
		
		Coordinates get_ko_coordinates() { return ko_coordinates; } ;
		Coordinates get_retake_coordinates() { return retake_coordinates; } ;
		Stone_color get_stone_color() { return stone_color; } ;
		void set_ko_coordinates( Coordinates ko_coordinates_ ) { ko_coordinates = ko_coordinates_ ; } ;
		void set_retake_coordinates( Coordinates retake_coordinates_ ) { retake_coordinates = retake_coordinates_; } ;
		void set_stone_color( Stone_color stone_color_ ) { stone_color = stone_color_; } ;

		void set_active ( bool active_)  { active = active_;
																			 if ( active == false ) 
																				 retake_coordinates = Coordinates ( -1,-1);} 
		bool get_active ( )  { return active ; } 
};

typedef unsigned long long Hash_key;  /** This type must be 64 bit */

/*[C] when changing type change also a maximum value of Hash_key in get_random_Hash_key(), now it is ULLONG_MAX*/

/**\brief Function returning random number of Hash_key type*/
Hash_key get_random_Hash_key() ;  
	
/**\brief Class handling hash keys. 
 *
 * This class manages creating hash keys for positions on the board ( according to Zobrist hashing method ), class Hash_table manager manages hashing itself. */
class Hash_board_manager { 
private:
	Hash_key hash_board[B_HEIGHT][B_WIDTH][COLORS_NUM]; //!< Board for random keys, 2 is for stones color ... attacker and defender 
	int	Hash_key_length; //!< Length of Hash key.
	Hash_key position_hash; // !< this is a hash value of actual position ... tightly connected to Board_manager
public: 

	Hash_board_manager();
	/**\brief Returns desired value from board of hash keys.*/
	Hash_key get_value ( Coordinates coordinates_ , Stone_color stone_color_ ) { return hash_board[ coordinates_.get_co_row()]
										 [coordinates_.get_co_column()][stone_color_];}
	Hash_key get_position_hash() { return position_hash; } 
	void set_position_hash( Hash_key new_value_ ) { position_hash = new_value_; }  // only when updating from nnplay_manager
	
	/**\brief Testing function.*/
	void print_position_hash();

	/**\brief Testing function.*/
	void print_hash_board();

	/**\brief Testing function.*/
	void print_info() ;

	/**\brief Initialization of board.*/
	void init_hash_board();

	/**\brief Creating new position hash from the old one.*/
	void xor_with_position_hash( Coordinates , Stone_color ); 
	
};
/* SWITCHED OFF BITSET
typedef bitset < B_HEIGHT * B_WIDTH> Board_bitset ;/**/ 

/**\brief Unplaying moves
 * 
 * This class contains some data structures neccessary for correct stones unplaying ( some local data structures e.g. #ko_manager,#pot_eyes_map, ...  serve only for copying identical data structures from Board_manager ). Unplaying itself is done by Board_manager::unplay_last_stone.*/
struct Unplay_manager {
	private:
		Block * neigbour_blocks [FOUR]; //!< pointers to neigbour blocks before the stone was played
		bool captured_neigbour_blocks [FOUR]; //!< pointers to neigbour blocks before the stone was played
		Coordinates stone_to_unplay; //!<coordinates of stone to unplay itself
		int number_stones_de; //!< it is neccessary to store number of defender's stones
		Hash_key position_hash; //!< actual position hash key 
		Ko_manager ko_manager;  //!< copy of Board_manager::ko_manager
		Pot_eyes_map pot_eyes_map; //!< copy of Board_manager::pot_eyes_map
		Playable_map playable_map; //!< copy of Board_manager::playable_map
		/* SWITCHED OFF BITSET 
		Board_bitset bitset_at;
		Board_bitset bitset_de;
			/**/

		friend class Board_manager;
	public:
		
		/**\brief Testing function.*/
		void info();
		
		/**\brief constructor
		 *
		 * Here array of neigbour blocks is initialized into NULL's.*/
		Unplay_manager() { for ( int i = 0; i < FOUR ; i++ ) 
													neigbour_blocks[i] = NULL; }   // VERY IMPORTANT !!! :
													
};

/**\brief Dynamic structure for unplaying moves.
 *
 * This class creates a dynamic structure of Unplay_manager's where each Unplay_manager represents one move to be unplayed. When there is played new
 * move, new item of Unplay_node is created appropriate Unplay_manager is filled with actual information and it is stored in the end of the list.*/

struct Unplay_node {   // dynamic list of unplay managers
	private: 
		Unplay_node * next; 	//!< pointer to next in the list 
		Unplay_manager unplay_manager; //!< Unplay_manager for actual move.
		friend class Board_manager;
	public:
};

typedef Point_status Point_board [B_HEIGHT][B_WIDTH];

/*\brief Managing under the stones play.
 * 
 * This class handles problem of playing again into place where defender's stones were captured. So far is  doing nothing special.
class Under_the_stones_manager{
private:
	bool active; //!< In actual move is potential under the stones capture
public:
	Under_the_stones_manager() { active = false; }
	void deactivate() { active=false;} 
	void activate() { active=true;} 
	bool get_active() { return active;}
};

ostream & operator<< ( ostream & out, Under_the_stones_manager A );*/

		
typedef Cross_point  Cross_point_board[B_HEIGHT][B_WIDTH];


/** \brief Class conduncting playing and unplaying moves.
 *
 * This class maintains position presentation, contains data structures neccessary for correct moves (un)playing and provides functions performing 
 * different tasks upon position. It also contains several classes defined previously in this header file ( e.g. Ko_manager, Unplay_node_handle , ... ) */
		
class Board_manager { 
	private:
		int next_block_id;       //!< New block will have this id.
		Cross_point_board board; //!< Board which pointers to block of stones.
		Playable_map playable_map; //!< Here are stored crosspoints that are playable ( == empty ) with their playing priorities.
		stringstream log_stream; //!< logging purposes	
			
		/*when hashing, key is sufficient to recognize position */
		/* SWITCHED OFF BITSET``
		Board_bitset bitset_at;  //!< bitset of attacker's stones
		Board_bitset bitset_de;  //!< bitset of defender's stones/**/

	public:
		Pot_eyes_map pot_eyes_map; //!< In the beginning potential eyes are stored here and then are updated in Eyes_manager.
		//Under_the_stones_manager under_the_stones_manager; 
		bool problematic_capture; //!< Helping variable to manage under the stones problems.
		Hash_board_manager hash_board_manager; //!< Instance of hash board manager taking care of board of hash keys.
		Ko_manager ko_manager; //!< Instance of Ko_manager.
		int number_stones_de; /*Number of defenders stone - when very low ( see T_searcher::search() ) group is captured.*/
		Point_board point_board ; //!< Alternative board representation used in Eye_manager 
		Unplay_node * unplay_node_handle; //!< Beginning of dynamic list of moves to be unplayed.	
		Coordinates_set under_the_stones_set; //!< Coordinates under captured stones defender's stones ( potential under the stones coordinates )
		
		/**\brief Initialization function.*/
		void init(); 

		/**\brief Creates new empty block. */
		Block * create_new_block ( Stone_color );

		/**\brief Captures block of stones. */
		void capture_block ( Block * );
		
		/**\brief Removes block of stones. */
		void remove_block ( Block * ) ;						

		/**\brief Plays a stone and updates data structures. */
		bool play_a_stone( Stone_color, Coordinates); 

		/**\brief Merges two block into one.*/
		void merge_blocks( Block * , Block * );
		
		/**\brief Testing function.*/
		void info();							

		/**\brief Testing function.*/
		void print_blocks_info ();

		/**\brief Testing function.*/
		stringstream & print_playable_map( bool print_for_sgf_printer);

		/**\brief Testing function.*/
		stringstream & print_pot_eyes_map( bool print_for_sgf_printer);

		/**\brief Testing function.*/
		void print_to_sgf( ostream & ); 

		/**\brief Testing function.*/
		void print_stones();

		/**\brief Testing function.*/
		void print_actual_unplay_info();

		/**\brief Testing function.*/
		void print_complete_unplay_info();
		
		/* SWITCHED OFF BITSET``
		void print_bitsets_info();/**/
		
		/**\brief Initialization from temp_board_manager.*/
		void init_from_temp_board_manager ( Temp_board_manager  );
		
		/**\brief #Point_board initialization.*/
		void fill_point_board();
		
		/**\brief Returns board manager to the conditions before last stone was played]*/
		void unplay_last_stone (); 

		Point_board & get_point_board() { return point_board; };
		Playable_map & get_playable_map() { return playable_map; };
		Pot_eyes_map * get_pot_eyes_map_pt() { return & pot_eyes_map; };
		Coordinates_set & get_under_the_stones_set() { return under_the_stones_set; };

		/**\brief Checks whether larger defender's group is not in atari.*/
		bool check_self_atari (  Stone_color stone_color_ , Coordinates coordinates_, int * );  // checks whether playing at coordinates isn't playing to atari

//	void check_under_the_stones_moves();

		/**\brief Checks whether given coordinates are adjacent*/
		bool check_coordinates_adjacency ( Coordinates , Coordinates ); 

		/**\brief Checks whether the moves takes ko.*/
		pair < Coordinates , bool > check_takes_ko ( Stone_color ,Coordinates); 

		/**\brief Checks whether a move captures something*/
		bool check_capture ( Stone_color , Coordinates , bool * , int * = NULL );

		/**\bried Checks whether a move is an atari*/
		bool check_atari ( Stone_color , Coordinates ); 

		/**\brief Important function checking whether a played move takes away liberty ( of defender or unsafe attacker ) */
		bool check_take_liberty ( Stone_color , Coordinates , int , Block ** = NULL );
		
		/**\brief Checks whether a move removes safe attacker liberty */
		bool check_take_liberty_of_safe_attacker ( Stone_color , Coordinates ); 

		
		/**\brief Checks connection of two blocks of same color */
		bool check_connection ( Stone_color stone_color_ , Coordinates coordinates_ , bool ) ;
		
		/**\brief Checks connection of unsafe attacker block to safe attacker*/
		bool check_save_connection ( Stone_color , Coordinates ); // [F] does a move of Stone_color at Coordinates connects to safe group ?  

		/**\brief Checks connection of eyes from the set*/
		bool check_eyes_connection ( Coordinates_set & );

		/**\brief Checks whether block has some unsafe neighbour of opposite color*/
		bool check_unsafe_neighbour ( Block * );
		
		
		/*SWITCHED OFF BITSET 
		
		void add_stone_to_bitset ( Coordinates, Stone_color );
		void remove_stone_from_bitset ( Coordinates, Stone_color );

		Board_bitset get_bitset_at ( ) { return bitset_at; } 
		Board_bitset get_bitset_de ( ) { return bitset_de; } 
		/**/
};


extern Ref_board ref_board;
extern Block block;
extern Board_manager board_manager;



#endif
