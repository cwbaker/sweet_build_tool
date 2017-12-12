//
// Scanner.cpp
// Copyright (c) 2007 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Scanner.hpp"
#include "Pattern.hpp"
#include <sweet/assert/assert.hpp>
#include <algorithm>

using std::max;
using namespace sweet;
using namespace sweet::build_tool;

Scanner::Scanner( unsigned int patterns_reserve, BuildTool* build_tool )
: build_tool_( build_tool ),
  initial_lines_( 0 ),
  later_lines_( 0 ),
  patterns_()
{
    SWEET_ASSERT( build_tool_ );
    patterns_.reserve( patterns_reserve );
}

/**
// Set the maximum number of unmatched lines allowed at the start of a file
// before scanning is stopped.
//
// @param initial_lines
//  The maximum number of unmatched lines allowed at the start of a file.
*/
void Scanner::set_initial_lines( int initial_lines )
{
    initial_lines_ = max( 0, initial_lines );
}

/**
// Get the maximum number of unmatched lines allowed at the start of a file 
// before scanning is stopped.
// 
// @return
//  The maximum number of unmatched lines allowed at the start of a file.
*/
int Scanner::get_initial_lines() const
{
    return initial_lines_;
}

/**
// Set the maximum number of unmatched lines allowed after at least one 
// matched line before scanning is stopped.
//
// @param later_lines
//  The maximum number of unmatched lines allowed after at least one matched 
//  line.
*/
void Scanner::set_later_lines( int later_lines )
{
    later_lines_ = max( 0, later_lines );
}

/**
// Get the maximum number of unmatched lines allowed after at least one 
// matched line before scanning is stopped.
// 
// @return
//  The maximum number of unmatched lines allowed after at least one matched 
//  line.
*/
int Scanner::get_later_lines() const
{
    return later_lines_;
}

/**
// Add a pattern to be matched to this Scanner.
//
// @param regex
//  The regular expression to search for.
//
// @param lua
//  The Lua object that holds the Lua virtual machine that \e lua_State is
//  part of.
//
// @param lua_state
//  The lua_State whose stack has the match function to call.
//
// @param position
//  The position on the stack to get the match function from.
*/
void Scanner::add_pattern( const std::string& regex, lua::Lua& lua, lua_State* lua_state, int position )
{
    patterns_.push_back( Pattern(regex, lua, lua_state, position) );
}

/**
// Get the patterns in this Scanner.
//
// @return
//  The patterns in this Scanner.
*/
const std::vector<Pattern>& Scanner::get_patterns() const
{
    return patterns_;
}