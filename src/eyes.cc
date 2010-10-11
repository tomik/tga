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

#include "eyes.h"

/**\file eyes.cc
 * \brief Contains implementation of methods related to statical eye recognition( \ref page_4 ).
 *
 * In this file methods performing eye recognition are implemented. Eye recognition is generally pretty simple and is a good  
 */
ostream & operator<< ( ostream & out, Group_status A ) { 

	switch ( A ) {
					case gs_alive:    out << "alive" ; break;
					case gs_dead:			out << "dead" ; break;
					case gs_unknown:  out << "unknown" ; break;
					case gs_ko: 			out << "ko" ; break;
					default : out << " invalid status ";
				}
		return out;
}
/** 
 * \param[in] A Status of the first group.
 * \param[in] B Status of the second group.
 * return True if first status < second status otherwise returns false.
 *
 * Definition of operator< : gs_dead < gs_unknown < gs_ko < gs_alive 
 * This operator is neccessary for comparing group statuses in minimax algorithm in T_searcher::search
 * However operator is defined for gs_unknown as well it should not be used for gs_unknown ( that is a 
 * special status needing individual solution ) */

bool operator< ( Group_status B , Group_status A ) { 

	 return (int ) B < (int ) A ; 

}

/**
 * \param[in] coordinates_ Potential eye coordinates. 
 * \param[in] pount_board_ reference to Board_manager::point_board, this is neccessary for deciding an 
 * #Eye_status of  potential eye.
 * \param[in] check_diagonal_eye This boolean switch is used for recognition of "diagonaly related eyes".
 *
 * This function resolves #Eye_status of potential eye at given coordinates_.
 * */
Eye_status Eye_manager::get_eye_status ( Coordinates coordinates_ , Point_board & point_board_ , bool check_diagonal_eye ) {
	if ( point_board_[coordinates_.get_co_row()][coordinates_.get_co_column()] == ps_at || 
			 point_board_[coordinates_.get_co_row()][coordinates_.get_co_column()] == ps_de )  
		return es_false_eye; /*[C] eye point is filled with a stone*/
	
	bool empty_eye_point; // this variable helps to resolve whether eye point has an es_produces_no_eye status 
	if 	( point_board_[coordinates_.get_co_row()][coordinates_.get_co_column()] != ps_uns_at ) /*pot_eye_point is empty*/  
		 empty_eye_point = true;
		else
		 empty_eye_point = false;
	

	bool unknown_status = false; /*[C] when set to true means that status is unknown or worse ... check for the false eye and then return unknown*/
	
	Coordinates pom_coordinates;

	/*[C] checking whether adjacent coordinates are allright*/
	for ( int i = 0; i < 4 ; i++ ) { 
		pom_coordinates = get_adjacent_coordinates ( coordinates_ , Direction(i) ) ;
		if ( pom_coordinates == Coordinates ( -1 , -1 ) ) 
			continue;
		if ( point_board_[pom_coordinates.get_co_row()][pom_coordinates.get_co_column()] == ps_at ) /*safe attacker at adjacent coordinate -> false eye*/
				return es_false_eye;
		if ( point_board_[pom_coordinates.get_co_row()][pom_coordinates.get_co_column()] != ps_de ) /*not sure eye, but if false proves return false */
				unknown_status = true;
		if ( point_board_[pom_coordinates.get_co_row()][pom_coordinates.get_co_column()] == ps_uns_at && empty_eye_point ) 
		return es_produces_no_eye; // [C] empty point adjacent to unsafe attacker cannot be an eye*/
	}
	
	bool pot_diag_eye = false ; // special eye ... two neigbouring across diagonal
	Coordinates diag_eye_coordinates; // coordinates of diagonal point ... brother eye
	
	int diag_safe_at = 0; /*[C] number of safe attacker stones at diagonals, if > 1 => false eye*/
	int diag_out_of_board = 0; /*[C] some diagonal point is out of board => if ( safe_diag_eye >= 1 ) => false_eye*/
	int diag_de = 0; /*[C] number of empty diagonal points*/
	
	for ( int i = 0; i < 4 ; i++ ) { 
		pom_coordinates = get_diagonal_coordinates ( coordinates_ , Direction(i) ) ;	
		if ( pom_coordinates == Coordinates ( -1 , -1 ) ) {
			diag_out_of_board++; /* in the center => 0 on the side => 2 in the corner => 3 no other values are possible */
			continue; 
		}
			if ( point_board_[pom_coordinates.get_co_row()][pom_coordinates.get_co_column()] == ps_de )
				diag_de++;
		if ( ( point_board_[pom_coordinates.get_co_row()][pom_coordinates.get_co_column()] == ps_empty ) 
					&& ( pot_diag_eye == false ) ) {	//[C] possible diagonal eye
				diag_eye_coordinates = pom_coordinates;
				pot_diag_eye = true;
		} 
				
		if ( ( point_board_[pom_coordinates.get_co_row()][pom_coordinates.get_co_column()] == ps_at ) ) 
			diag_safe_at++; /*safe diagonal attacker*/	
	}	// for

	if ( diag_safe_at >= 2 || (diag_safe_at >=1 && diag_out_of_board >=1 ) )  
		return es_false_eye;

	if ( unknown_status ) 
		return es_unknown;

	if ( diag_de + diag_out_of_board == 4 || ( diag_de == 3 && diag_out_of_board == 0 ) ) /*[C] definition of full eye*/
		return es_full_eye;
	
	if ( diag_de + diag_out_of_board == 3 || ( diag_de == 2 && diag_out_of_board == 0 ) )  /*[C] time to check for diagonal eye*/
		if ( pot_diag_eye )  { // possible diagonal eye
			if ( check_diagonal_eye ) // both diagonal points are eyes => 
				return es_full_eye; 
			else {   /*[C] this is first diagonal eye*/
				if  ( (*this). get_eye_status ( diag_eye_coordinates, point_board_ , true  ) == es_full_eye )
					return es_full_eye;
				else
					return es_unknown;
			} 
		}
	
		return es_unknown;
				
}

/**
 * \param[in] point_board_ Reference to Board_manager::point_board this is neccessary for deciding an #Eye_status 
 * of  potential eye.
 * \param[in] pot_eye_map_pt Pointer to Board_manager::pot_eyes_map, this is a list of potential eyes which are 
 * further searched in Eye_manager::get_eye_status. Function might delete items from this map ( therefore pointer is 
 * used ) : when a false eye is found it is deleted.
 *
 * This functions applies Eye_manager::get_eye_status on every item from (pot_eye_map_pt*) and from retrieved
 * information decides whether the group status is alive/dead or unknownd. When two full eyes are found checking their 
 * connectivity is included by calling function Board_manager::check_eyes_connection
 * */
Group_status Eye_manager::get_group_status( Point_board & point_board_ , Pot_eyes_map * pot_eye_map_pt_) {
int full_eyes_num = 0;
int pot_eye_count = 0;
int produces_no_eye_num = 0;  /*[C] number of crosspoints producing no eye*/
Coordinates_vector false_eyes_vector; /* [C] vector of false eyes, these points are excluded from potential eyes */ 
Eye_status eye_status_;
Coordinates coordinates_1 = Coordinates( -1,-1);
Coordinates coordinates_2 = Coordinates( -1,-1);


(*this).full_eyes_set.clear();

	  for ( Pot_eyes_map::iterator it = pot_eye_map_pt_->begin() ;it != pot_eye_map_pt_->end() ; it++ )  {
			pot_eye_count++; // counts potential eyes
		
			eye_status_ = get_eye_status( (*it).first , point_board_ , false); // [C] Retrieve Eye status.
			if ( eye_status_ == es_full_eye) {
					(*this).full_eyes_set.insert ( (*it).first); //[C] Storing full eye into the set prevents defender playing there.
					full_eyes_num++;
					continue;
			}
			if ( eye_status_ == es_false_eye ) { //[C] false eye -> will be removed from the map
					false_eyes_vector.push_back ( (*it).first);
					continue;
			}
			if ( eye_status_ == es_produces_no_eye ) { // [C] not false eyes, but virtually producing no eye
						produces_no_eye_num++;
						continue;
			}
			// ordinary potential eye - saving 2 potential eyes for potential heuristic when 2 adjacent potetial eyes produces only one eye.
			if ( coordinates_1 == Coordinates (-1,-1 ) )
					coordinates_1 = (*it).first;
			else 
					coordinates_2 = (*it).first;

	
		} //for 
		
			if ( full_eyes_num >= 2 ) 
				if ( board_manager.check_eyes_connection ( full_eyes_set ) ) /*[C] at least two eyes must be connected */
					return gs_alive;
			
		/*[C] Erasing false eyes from the list of potential eyes*/
	  for ( Coordinates_vector::iterator it = false_eyes_vector.begin() ;it != false_eyes_vector.end() ; it++ )  
			pot_eye_map_pt_->erase ( *it );
	
		/*[C] Too little potential eyes => gs_dead*/
		if 	( pot_eye_map_pt_->size() - produces_no_eye_num < 2 )  
			return gs_dead;

		if ( ( pot_eye_map_pt_->size() - produces_no_eye_num ) == 2 )
			if ( board_manager.check_coordinates_adjacency ( coordinates_1 , coordinates_2 ) )
						return gs_dead;

		/*[C] otherwise unknown status*/
  	return gs_unknown;
}
