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

/* This header file declares class Eye_manager and other related classes which serves to identify eyes of the group. * */

#ifndef EYES_
#define EYES_

/** \file eyes.h
 * \brief Header file for eyes.cc.
 *
 * In this file class Eye_manager and other data structures related to recognizing eyes of the group are declared.
 * */

#include <string>
#include <set>
#include <vector> 
#include <iostream>

#include "constants.h"
#include "board.h" // because of Coordinates 

using namespace std;

/**\brief Enum of possible group statuses.
 * 
 * Group statuses in enum are ordered from the worst to the best ( from defender's point of view ) 
 * and there is also very simple operator< defined upon this enum.
 */
enum Group_status { 
	gs_dead , //!< Group is unconditionally dead 
	gs_unknown , //!< Status of the group is yet unknown 
	gs_ko, //!< Group is alive/dead in ko
	gs_alive  //!< Group is unconditionally alive 
};

/**\brief Enum of possible eye statuses.
 *
 * Values from this enum are returne by function Eye_manager::get_eye_status() and according to them it
 * is decided whether the status of the group is alive/dead or unknown ( Eye_manager::get_group_status()).
 */
enum Eye_status { 
	es_full_eye,  //!< This is completed eye.
	es_false_eye,  //!< This might not be an eye in future.
	es_produces_no_eye, //!< This might not be marked as a false eye ( and excluded for future search ) , but virtually produces no eye ( e.g. empty point adjacent to uns_at stone cannot produce an eye right now )
	es_unknown  //!< Yet unknown.
};


ostream & operator<< ( ostream & out, Group_status A ) ;

/*\brief Operator < for Group statuses.*/
bool operator< ( Group_status , Group_status );  

/**\brief Handles searching for eyes of the defender's group.
 *
 * This class provides functions for simple statical eye evaluation.
 */

class Eye_manager {
	private: 
	public: 
		Coordinates_set full_eyes_set; //!< When get_group_status() function founds out some full eyes it is put in this set and defender is prevented to fill it => effective forward pruning ( without information loss ) 
		
		/**\brief Checks #Eye_status of given potential eye.*/
		Eye_status get_eye_status( Coordinates coordinates , Point_board & point_board, bool check_diagonal_eye ); 

		/**\brief Basic function staticaly resolving status of the defenders group.*/		
		Group_status get_group_status(Point_board & , Pot_eyes_map *);
};

extern Eye_manager eye_manager;

#endif 
