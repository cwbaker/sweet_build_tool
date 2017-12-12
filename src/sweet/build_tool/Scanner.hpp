#ifndef SWEET_BUILD_TOOL_SCANNER_HPP_INCLUDED
#define SWEET_BUILD_TOOL_SCANNER_HPP_INCLUDED

#include "declspec.hpp"
#include "Pattern.hpp"
#include <string>
#include <vector>

namespace sweet
{

namespace build_tool
{

class BuildTool;

/**
// A collection of regular expressions and match functions that are used to 
// scan source files and the output of externally executed processes for 
// regular expressions.
*/
class SWEET_BUILD_TOOL_DECLSPEC Scanner
{
    BuildTool* build_tool_; ///< The BuildTool that this Scanner is part of.
    int initial_lines_; ///< The maximum number of unmatched lines to allow at the start of a file before stopping scanning.
    int later_lines_; ///< The maximum number of unmatched lines to allow after at least one match before stopping scanning.
    int maximum_matches_; ///< The maximum number of matches to find before stopping scanning.
    std::vector<Pattern> patterns_; ///< The Patterns that have been added to this Scanner.

    public:
        Scanner( unsigned int patterns_reserve, BuildTool* build_tool );
        ~Scanner();
        void set_initial_lines( int initial_lines );
        int initial_lines() const;
        void set_later_lines( int later_lines );
        int later_lines() const;        
        void set_maximum_matches( int maximum_matches );
        int maximum_matches() const;
        void add_pattern( const std::string& regex, lua::Lua& lua, lua_State* lua_state, int position );
        const std::vector<Pattern>& patterns() const;
};

}

}

#endif
