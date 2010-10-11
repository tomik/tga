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

#include <iostream>
#include <iomanip>
#include <time.h> 

/** \file main.h
 * \brief Header file to main.cc*/

#include "log_manager.h"
#include "board.h"
#include "p_manager.h"
#include "sgf_read.h"
#include "eyes.h"
#include "t_search.h"
#include "constants.h"

Log_manager log_m; //!< This is the only instance of Log_manager
Board_manager board_manager; //!< This is the only instance of Board_manager
Params_manager params_manager; //!< This is the only instance of Params_manager
Sgf_parser sgf_parser; //!< This is the only instance of Sgf_parser
Eye_manager eye_manager; //!< This is the only instance of Eye_manager
Ref_board ref_board; //!< This is the only instance of Ref_board
T_searcher t_searcher; //!< This is the only instance of T_searcher
Sgf_printer sgf_printer; //!< This is the only instance of Sgf_printer
