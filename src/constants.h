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

#ifndef KONSTANTY_
#define KONSTANTY_

//#define LLONG_MAX (-9223372036854775807-1)
//#define ULLONG_MAX 18446744073709551615ULL // neccessary to add ULL to the end !!!
//#define LLONG_MAX (-9223372036854775807-1)

#define FOUR 4 
#define COLORS_NUM 2 // number of stone colors == 3 black , white , none

#define TOO_LITTLE_TO_LIVE 4

#define POS_RECORD_BEGIN "###"
#define POS_RECORD_END   "$$$"
#define BIT_TAPE_LENGTH  121

#define EQ_POS 8 // number of ,,equivalent" positions arising from 1 by rotating/inverting

#define PT_CORNER "corner"
#define PT_SIDE		"side"
#define PT_CENTER	"center"
#define PT_ALL 		"all"

#define MAX_DYN_HEUR_DEPTH 10 // Maximal depth into which tree size heuristics works


#define NUMBER_OF_STONES_ALLOWED_TO_SELF_ATARI 2 //  in board.cc  Board_manager::check_self_atatri() // defender never needs to attari more than 3 stones

//#define TESTING 

#define DEPTH_CUT_OFF 10  // when to cut off the tree 

#define EXPECTED_HASH_KEY_LENGTH 64


enum Board_proportions {
	B_WIDTH = 19,
	B_HEIGHT= 19
		
}

/* This is the obsolete POSIX.1-1988 name for the same constant. */;
//# if !defined __STRICT_ANSI__ && !defined __USE_XOPEN2K
# ifndef CLK_TCK
# define CLK_TCK CLOCKS_PER_SEC
# endif
//# endif

#endif
