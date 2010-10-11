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

/*=============================================================================
Log_manager is class taking care of all thi Pattern matching

Pattern matching is handled by these modules ( and their header_files ) :

sgf_compiler.cc - contains classes covering compiling sgf positions into one ,,bit_tape" database
pos_db.cc - class Pos_db which represents compiled database itself ( after loaded in memory ) is implemented here
pattern_matcher.cc - using pos_db implements matching given position against compiled database

nks around logging - where to log,
priorities of logging, ...
==============================================================================*/

#ifndef LOG_MANAGER_  
#define LOG_MANAGER_


#include <string>
#include <iostream>
#include <sstream>

using namespace std;

enum Log_priority {
	low,
	normal,
	high,
	very_high,
	output 
} ;

class Log_manager { 
	private: 
		ostream * log_ostream; 
		Log_priority accept_priority; 
		bool active;
	public: 
		stringstream buff;
		Log_manager() { log_ostream = & cout; active = true;};
		Log_manager( ostream & log_ostream_) { log_ostream = & log_ostream_ ; }
		void set_active ( bool active_ ) { active = active_; }
		void set_accept_priority ( Log_priority accept_priority_ ) { accept_priority = accept_priority_; }
		void print_separator () { if (active ) 
																(*log_ostream) << "=======================================" << endl; }
		void flush_buffer_imp() {  if (active ) {
																	print_separator(); //important texts 
																	flush_buffer();
   																print_separator();
																}
											 			}
		void flush_buffer() { if ( active ) {
														(*log_ostream) << buff.str();
														buff.str("");
													}
												} /* neccessary - to empty buffer */
};

extern Log_manager log_m; 

#endif
