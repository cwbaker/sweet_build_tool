#ifndef FORGE_FILTER_HPP_INCLUDED
#define FORGE_FILTER_HPP_INCLUDED

struct lua_State;

namespace sweet
{

namespace forge
{

/**
// Hold a reference to a function in Lua so that it doesn't get garbage 
// collected.
*/
class Filter
{
    lua_State* lua_state_;
    int reference_;
    
public:
    Filter();
    Filter( lua_State* lua_state, lua_State* calling_lua_state, int position );
    Filter( const Filter& value );
    Filter& operator=( const Filter& value );
    ~Filter();
    int reference() const;
};

}

}

#endif