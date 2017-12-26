#ifndef SWEET_BUILD_TOOL_GRAPHWRITER_HPP_INCLUDED
#define SWEET_BUILD_TOOL_GRAPHWRITER_HPP_INCLUDED

#include <vector>
#include <string>
#include <ostream>
#include <memory>
#include <ctime>

namespace sweet
{

namespace build_tool
{

class Target;

class GraphWriter
{
    std::ostream* ostream_;

public:
    GraphWriter( std::ostream* ostream );
    void write( Target* root_target );
    void object_address( const void* address );
    void value( bool value );
    void value( int value );
    void value( std::time_t value );
    void value( const std::string& value );
    void value( const char* value, size_t size );
    void value( const std::vector<std::string>& values );
    void value( const std::vector<Target*>& values );
    void refer( const std::vector<Target*>& references );
};

}

}

#endif
