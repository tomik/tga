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

#ifndef T_SEARCH_
#define T_SEARCH_
 
/**\file t_search.h
 * \brief Header file for t_search.cc
 *
 * In this header file classes and data structures neccessary for proper tree searching are declared. 
 * \include t_search.h
 */
#include <string>
#include <set>
#include <vector> 
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#include "log_manager.h"
#include "p_manager.h"
#include "constants.h"
#include "board.h"
#include "eyes.h"



using namespace std;


/** this enum tells me whether am i in maximizing or minimizing node */ 

enum Node_type { 
 nt_min,  
 nt_max  
};

/** Possible values accuracy of group status. */
enum Group_status_accuracy {  
	gsa_certain, //!< Certain status ( represents correct evaluation of the group).
	gsa_depth_uncertain, //!< Uncertain due to depth cutoff ( possible global information loss).
	gsa_alfa_beta_uncertain, //!< Uncertain due to alfa beta cutoff ( without global information loss).
	gsa_unknown // !< Accuracy is unknown.
};

ostream & operator<< ( ostream & out, Group_status_accuracy A ) ;

class T_searcher ;
	
/**\brief Class managing creating output sgf file.
 *
 * This class stores information about sgf output ( filename , fstream , ... ) and provides functions to for operating 
 * the output sgf file.
 * */
class Sgf_printer { 
	private:
		ofstream sgf_out; //!<This is ofstream variable represents sgf output file.
		friend class T_searcher;
	public:
		stringstream buff; //!<This buffer is used for iostream operations and afterwards is flushed to Sgf_printer::sgf_out

		/**\brief Assigns output file to the Sgf_printer::sgf_out
		 * \param[in] file_name Filename of the output file.
		 *
		 * This method assings Sgf_printer::sgf_out file to print the output to. If there is not a file of given file_name, then it is created.
		 * */
    void set_output ( string file_name ) {
			if ( sgf_out.is_open() ) 
				sgf_out.close();
			sgf_out.open(file_name.c_str() , fstream::in | fstream::out | fstream::trunc);	
			sgf_out.flush();
			if ( ! sgf_out.good() )
				cout << "CHYBA" <<endl; // CHANGE !!! create file 
		};
		
		/**\brief Prints initial position to the sgf output. */
		void print_initial_position ( Point_board & point_board );
		
		/**\brief Prints sgf header. */
		void print_sgf_header( Stone_color );  /*[C] argument says who's turn it is */

		/**\brief Prints string to file. */
		void print_to_file ( string str );

		/**\brief Transforms coordinates to string. */
		string coordinates_to_string ( Coordinates  );
		
		/**\brief Prints comment with all neccessities.*/
		void print_comment ( bool ); /*Variable says if this comment might be blocked by an argument option ,,blockable" true == migh be blocked */

};

class Hash_table_manager;
class Hash_table_item;

bool corresponds ( Hash_table_item & , Hash_table_item & ) ;

/**\brief Represents one item in the Hash_table_manager::hash_table.
 *
 * This class contains all neccessary information to represent position in transposition tables ( #hash_table ) 
 * and also useful information which helps to cutoff search after position is found in transposition tables.
 * */

class Hash_table_item{ 
private:   
	Hash_key hash_key; //!< This is ( usually ) 64 bit key representing position ( it also differs same position with different players to move )
	Coordinates ko_coordinates; //!< on this coordinates it is forbidden to play for player on the turn ( opponent took ko with last move ) 
	Group_status new_alfa;  //!< This information is retrieved when an identical position to actual is found in transposition tables
	Group_status new_beta;  //!< This information is retrieved when an identical position to actual is found in transposition tables
	Stone_color stone_color_to_move; //!< Used for key normalization.
	Group_status_accuracy accuracy; //!< How is stored position accurate
	short depth; // !< how was search depth limited in time of saving position 
	Node_type node_type; //!< type of the node of position
	/*SWITCH OFF BITSET 
	Board_bitset bitset_at;
	Board_bitset bitset_de;
	/**/
	Group_status alfa; //!< Input value of alfa.
	Group_status beta; //!< Input value of beta. These are neccessary for comparing two positions ( whether status from one might be used in the other)
	Hash_table_item * next; //!< Pointer to next item ( when more items are hashed to one position in the table, they form a dynamic list ).
	friend class Hash_table_manager;
	friend bool corresponds ( Hash_table_item &A, Hash_table_item &B);
public:
	/**\brief constructor*/
	Hash_table_item() { next = NULL ;};
		
	/**\brief Constructor with intial values of properties.*/
	Hash_table_item(Hash_key hash_key_, Stone_color stone_color_to_move_ , Coordinates ko_coordinates_,
																	/*SWITCH OFF BITSET Board_bitset bitset_at_ , Board_bitset bitset_de_ ,/**/ int depth_ , Node_type node_type_,
																	Group_status alfa_ , Group_status beta_ , Group_status_accuracy accuracy_ = gsa_unknown , 
																	Group_status new_alfa_ = gs_unknown , Group_status new_beta_ = gs_unknown ) : 
		
									hash_key (hash_key_ ), stone_color_to_move ( stone_color_to_move_ ), ko_coordinates ( ko_coordinates_ ), depth ( depth_ ), 
									node_type ( node_type_ ), new_alfa ( new_alfa_ ), new_beta ( new_beta_ ) , accuracy ( accuracy_ ), 
									/*SWITCH OFF BITSET bitset_at ( bitset_at_ ),bitset_de ( bitset_de_) ,/**/  alfa ( alfa_ ) , beta ( beta_ )	{ next = NULL;};
};

/**\brief Manages transposition tables.
 *
 * This class contains transposition tables ( #hash_table) and  miscellanous methods / properties to operate these tables : 
 *  - create and init transposition tables
 *  - inserting new Hash_table_item into tables 
 *  - retrieving existing item ( according to corresponds() function ) 
 *  
 * */
	
class Hash_table_manager {
private:
	bool active;   // !< States whether are transposition tables active or not. 
	Hash_table_item ** hash_table; //!< Transposition table itself.
	
	int hash_table_size; //!< Size of the table ( in number of positions for primary hash ).
	int hash_table_size_log;  //!< logarithm of #hash_table_size
	int hash_key_length; //!< Lenght of the hash key in bits ( usually 64 ).
	int shift_num; //!< when hashing, hash key is shifted to the right by shift num to get better ,,random performance" ( higher bits are more random )

	int items_num; //!< Number of actually stored items

	
	int collision_num; //!< Number of collisions/
	int longest_row; //!< Longest row of items at some particular position in #hash_table.

	Hash_key at_to_move_normalisation;  //!< Random hash key used to normalize position ( xor ) when is attacker to move.
	Hash_key de_to_move_normalisation;  //!< Random hash key used to normalize position ( xor ) when is defender to move.

public:
	/**\brief constructor*/
	Hash_table_manager() { active = false; collision_num = 0; hash_key_length = 8*sizeof( Hash_key ); /*hash_table = NULL;*/};

	/**'brief Activates table*/
	void hash_table_activate();

	void hash_table_init_testing_variables(){ collision_num = 0 ; longest_row;} ;  /*[C] neccessary when save_tt is not switched 0 */
	
	void hash_table_deactivate() {  active = false; delete (hash_table); } ; 

	/**\brief Normalizes hash_key according to color.*/
	Hash_key get_normalized_hash_key ( Hash_key , Stone_color);

	/**\brief Stores item into #hash_table.*/
	void store_item(Hash_table_item);
	
	/**\brief Retrieves item's status from #hash_table.*/
	pair < Group_status, Group_status > get_item_status( Hash_table_item, bool &, Group_status_accuracy &); // [F] tries to find item in hash table which has same values as given item  ( key , stone color , ... ) 

	bool actualize_item( Hash_table_item *, Hash_table_item &  );

	/**\brief Prints short info.*/
	void print_info();
};/**/


typedef set < Hash_key >  Hash_repset;
/*Hash key is uniq for every item of hash_repmap => there MIGH NOT happen that in the repmap are 2 items with same key and different level*/

/**'\brief Repetititons manager.
 *
 * This class stores positions along the path to actual node and then controls ( and prevents ) possible position repetitions.*/

class Hash_repset_manager {
	private: 
		Hash_repset hash_repset; //!< Set of Hash_keys.
	public:


		/**\brief Stores hash key.
		 * \param[in] hash_key Hash key of actual position.
		 * \return Returns whether key was saved.
		 *
		 * This method inserts input hash key ( representing actual position ) into set of positions ( it is presumed, that key is normalized 
		 * according to color of actual player. When the key is not saved, function returns false ( that means a repetition ) otherwise returns true.
		 * */
		bool store_item ( Hash_key hash_key_ ) { return ( hash_repset.insert ( hash_key_ )).second ;};
		
		/**\brief Removes hash key.
		 * \param[in] hash_key Hash key of actual position.
		 *
		 * This method removes hash key from set of possible repetition positions ( it is expected that the key is normalized ).  
		 */
		void remove_item ( Hash_key hash_key_ ) { hash_repset.erase ( hash_key_ ); };
};
/**\brief Structure returned by tree search
 * */
struct Search_result { 
	Group_status group_status; 
	Group_status_accuracy group_status_accuracy;
	int tree_size;
	
	Search_result () {}; 
	Search_result ( Group_status group_status_ , Group_status_accuracy group_status_accuracy_ , int tree_size_ ) :
	group_status ( group_status_ ) , group_status_accuracy ( group_status_accuracy_ ) , tree_size ( tree_size_ ) {};
};

/**\brief Dynamic heuristic item 
 *
 * This struct represents one item in dynamic heuristic structure. It consists of coordinates ( representing played ) move and related integer 
 * ( representing heuristic value ).
 */
struct Dyn_heur_item { 
	Coordinates coordinates; //!< coordinates of move 
	int h_value; //!< heuristic value 	
	Dyn_heur_item() {};
	Dyn_heur_item( Coordinates coordinates_ , int h_value_ ) : coordinates ( coordinates_ ) , h_value ( h_value_ )  {};
};

typedef  map < Coordinates , int > Dyn_heur_map;

/**\brief Class managing tree search.
 *
 * T_searcher is class which manages searching the tree of possible positions, 
 * using standart minimax algorithm together with pruning techniques and heuristics. The most remarkable is it's recursive 
 * function T_searcher::search.
 * */


class T_searcher { 
private:
	int nodes_num; //!< Number of visited nodes during the actual iteration of tree search
	int total_nodes_num; //!< Numver of visited nodes during actual problem ( all iterations ) 
	bool ab_pruning;  //!< indicatis if the alfa beta pruning is activated
	Hash_repset_manager hash_repset_manager;
	Group_status uninteresting_status; //!< when \c -ps option is used, than certain status that might be pruned 
	int max_depth;   //!< how deep will he go 
	Dyn_heur_map dyn_heur[MAX_DYN_HEUR_DEPTH]; //!< Data structure for managing dynamic heuristics
	pair < Coordinates , int > dyn_heur_max[MAX_DYN_HEUR_DEPTH]; //!< Data structure for saving maxnimal ( best ) values of dynamic heuristic on each level
	
public:
	Hash_table_manager hash_table_manager; 
	
	/**\brief This method performs tree search.*/
	Search_result search (Node_type, int , Group_status , Group_status, bool , bool  );
	
	int get_nodes_num () { return nodes_num; }
	int set_total_nodes_num ( int nodes_num_ ) { total_nodes_num = nodes_num_;  } 
	
	int get_total_nodes_num () { return total_nodes_num; }
	void set_ab_pruning ( bool ab_pruning_ ) { ab_pruning = ab_pruning_; }
	T_searcher () { nodes_num = 0; 	}
	
	void init() { max_depth = 25; total_nodes_num = 0; 
		for ( int i = 0; i < MAX_DYN_HEUR_DEPTH; i++ )  { // emptying dynamic heuristic data structures
			dyn_heur[i].clear();
			dyn_heur_max[i].second = -1; 
			dyn_heur_max[i].first = Coordinates (-1,-1); 
		}
	}
	void increase_max_depth() { max_depth+=5;}
	int get_max_depth () { return max_depth; }

	/**\brief Setting values in dynamic heuristic ( tree size ) */
	void add_tree_size_to_dyn_heur ( int , Coordinates , int  );

	void set_dyn_heur_max( int level_ ,  Coordinates coordinates_ , int new_value_ ) { dyn_heur_max[level_] = make_pair ( coordinates_, new_value_ ) ;}
	

	/**\brief Searches son of actual node and returns found Group_status.*/
	Search_result search_son ( Coordinates , Node_type , int , Group_status , Group_status , bool ); /*[F] this function is used to make code more legible*/
	
};


extern T_searcher t_searcher;
extern Sgf_printer sgf_printer;

#endif
