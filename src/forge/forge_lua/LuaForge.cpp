//
// LuaBuildTool.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaBuildTool.hpp"
#include "LuaFileSystem.hpp"
#include "LuaSystem.hpp"
#include "LuaContext.hpp"
#include "LuaGraph.hpp"
#include "LuaTargetPrototype.hpp"
#include "LuaTarget.hpp"
#include "types.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/build_tool/Filter.hpp>
#include <sweet/build_tool/Arguments.hpp>
#include <sweet/build_tool/Scheduler.hpp>
#include <sweet/process/Process.hpp>
#include <sweet/process/Environment.hpp>
#include <sweet/luaxx/luaxx.hpp>
#include <sweet/assert/assert.hpp>
#include <lua.hpp>
#include <string>

using std::string;
using std::unique_ptr;
using boost::filesystem::path;
using namespace sweet;
using namespace sweet::lua;
using namespace sweet::luaxx;
using namespace sweet::build_tool;

LuaBuildTool::LuaBuildTool( BuildTool* build_tool )
: build_tool_( nullptr ),
  lua_state_( nullptr ),
  lua_file_system_( nullptr ),
  lua_context_( nullptr ),
  lua_graph_( nullptr ),
  lua_system_( nullptr ),
  lua_target_( nullptr ),
  lua_target_prototype_( nullptr )
{
    create( build_tool );
}

LuaBuildTool::~LuaBuildTool()
{
    destroy();
}

lua_State* LuaBuildTool::lua_state() const
{
    SWEET_ASSERT( lua_state_ );
    return lua_state_;
}

LuaTarget* LuaBuildTool::lua_target() const
{
    SWEET_ASSERT( lua_target_ );
    return lua_target_;
}

LuaTargetPrototype* LuaBuildTool::lua_target_prototype() const
{
    SWEET_ASSERT( lua_target_prototype_ );
    return lua_target_prototype_;
}

void LuaBuildTool::create( BuildTool* build_tool )
{
    SWEET_ASSERT( build_tool );

    destroy();

    build_tool_ = build_tool;
    lua_state_ = luaxx_newstate();
    lua_file_system_ = new LuaFileSystem;
    lua_context_ = new LuaContext;
    lua_graph_ = new LuaGraph;
    lua_system_ = new LuaSystem;
    lua_target_ = new LuaTarget;
    lua_target_prototype_ = new LuaTargetPrototype;

    static const luaL_Reg functions[] = 
    {
        { "set_maximum_parallel_jobs", &LuaBuildTool::set_maximum_parallel_jobs },
        { "maximum_parallel_jobs", &LuaBuildTool::maximum_parallel_jobs },
        { "set_stack_trace_enabled", &LuaBuildTool::set_stack_trace_enabled },
        { "stack_trace_enabled", &LuaBuildTool::stack_trace_enabled },
        { "set_build_hooks_library", &LuaBuildTool::set_build_hooks_library },
        { "build_hooks_library", &LuaBuildTool::build_hooks_library },
        { "execute", &LuaBuildTool::execute },
        { "print", &LuaBuildTool::print },
        { nullptr, nullptr }
    };

    luaxx_create( lua_state_, build_tool, BUILD_TOOL_TYPE );
    luaxx_push( lua_state_, build_tool );
    luaL_setfuncs( lua_state_, functions, 0 );
    lua_context_->create( build_tool, lua_state_ );
    lua_file_system_->create( build_tool, lua_state_ );
    lua_graph_->create( build_tool, lua_state_ );
    lua_system_->create( build_tool, lua_state_ );
    lua_target_->create( lua_state_ );
    lua_target_prototype_->create( lua_state_, lua_target_ );
    lua_setglobal( lua_state_, "build" );

    // Set `package.path` to load build scripts stored in `../lua` relative 
    // to the `build` executable.  The value of `package.path` may be 
    // overridden again in `build.lua` before requiring modules.
    path first_path = build_tool_->executable( "../lua/?.lua" );
    path second_path = build_tool_->executable( "../lua/?/init.lua" );
    string path = first_path.generic_string() + ";" + second_path.generic_string();
    lua_getglobal( lua_state_, "package" );
    lua_pushlstring( lua_state_, path.c_str(), path.size() );
    lua_setfield( lua_state_, -2, "path" );
    lua_pop( lua_state_, 1 );
}

void LuaBuildTool::destroy()
{
    delete lua_target_prototype_;
    lua_target_prototype_ = nullptr;

    delete lua_target_;
    lua_target_ = nullptr;

    delete lua_system_;
    lua_system_ = nullptr;

    delete lua_graph_;
    lua_graph_ = nullptr;

    delete lua_context_;
    lua_context_ = nullptr;

    delete lua_file_system_;
    lua_file_system_ = nullptr;

    if ( lua_state_ )
    {
        luaxx_destroy( lua_state_, build_tool_ );
        lua_close( lua_state_ );
    }

    lua_state_ = nullptr;
    build_tool_ = nullptr;
}

/**
// Extract assignments from \e assignments_and_commands and use them to 
// assign values to global variables.
//
// This is used to accept variable assignments on the command line and have 
// them available for scripts to use for configuration when commands are
// executed.
//
// @param assignments
//  The assignments specified on the command line used to create global 
//  variables before any scripts are loaded (e.g. 'variant=release' etc).
*/
void LuaBuildTool::assign_global_variables( const std::vector<std::string>& assignments )
{
    for ( std::vector<std::string>::const_iterator i = assignments.begin(); i != assignments.end(); ++i )
    {
        std::string::size_type position = i->find( "=" );
        if ( position != std::string::npos )
        {
            std::string attribute = i->substr( 0, position );
            std::string value = i->substr( position + 1, std::string::npos );
            lua_pushlstring( lua_state_, value.c_str(), value.size() );
            lua_setglobal( lua_state_, attribute.c_str() );
        }
    }
}

int LuaBuildTool::set_maximum_parallel_jobs( lua_State* lua_state )
{
    const int BUILD_TOOL = 1;
    const int MAXIMUM_PARALLEL_JOBS = 2;
    BuildTool* build_tool = (BuildTool*) luaxx_check( lua_state, BUILD_TOOL, BUILD_TOOL_TYPE );
    int maximum_parallel_jobs = static_cast<int>( luaL_checkinteger(lua_state, MAXIMUM_PARALLEL_JOBS) );
    build_tool->set_maximum_parallel_jobs( maximum_parallel_jobs );
    return 0;
}

int LuaBuildTool::maximum_parallel_jobs( lua_State* lua_state )
{
    const int BUILD_TOOL = 1;
    BuildTool* build_tool = (BuildTool*) luaxx_check( lua_state, BUILD_TOOL, BUILD_TOOL_TYPE );
    int maximum_parallel_jobs = build_tool->maximum_parallel_jobs();
    lua_pushinteger( lua_state, maximum_parallel_jobs );
    return 1;
}

int LuaBuildTool::set_stack_trace_enabled( lua_State* lua_state )
{
    const int BUILD_TOOL = 1;
    const int STACK_TRACE_ENABLED = 2;
    BuildTool* build_tool = (BuildTool*) luaxx_check( lua_state, BUILD_TOOL, BUILD_TOOL_TYPE );
    bool stack_trace_enabled = lua_toboolean( lua_state, STACK_TRACE_ENABLED ) != 0;
    build_tool->set_stack_trace_enabled( stack_trace_enabled );
    return 0;
}

int LuaBuildTool::stack_trace_enabled( lua_State* lua_state )
{
    const int BUILD_TOOL = 1;
    BuildTool* build_tool = (BuildTool*) luaxx_check( lua_state, BUILD_TOOL, BUILD_TOOL_TYPE );
    bool stack_trace_enabled = build_tool->stack_trace_enabled();
    lua_pushboolean( lua_state, stack_trace_enabled ? 1 : 0 );
    return 1;
}

int LuaBuildTool::set_build_hooks_library( lua_State* lua_state )
{
    const int BUILD_TOOL = 1;
    const int BUILD_HOOKS_LIBRARY = 2;
    BuildTool* build_tool = (BuildTool*) luaxx_check( lua_state, BUILD_TOOL, BUILD_TOOL_TYPE );
    const char* build_hooks_library = luaL_checkstring( lua_state, BUILD_HOOKS_LIBRARY );
    build_tool->set_build_hooks_library( string(build_hooks_library) );
    return 0;
}

int LuaBuildTool::build_hooks_library( lua_State* lua_state )
{
    const int BUILD_TOOL = 1;
    BuildTool* build_tool = (BuildTool*) luaxx_check( lua_state, BUILD_TOOL, BUILD_TOOL_TYPE );
    const string& build_hooks_library = build_tool->build_hooks_library();
    lua_pushlstring( lua_state, build_hooks_library.c_str(), build_hooks_library.size() );
    return 1;
}

int LuaBuildTool::execute( lua_State* lua_state )
{
    try
    {
        const int BUILD_TOOL = 1;
        const int COMMAND = 2;
        const int COMMAND_LINE = 3;
        const int ENVIRONMENT = 4;
        const int DEPENDENCIES_FILTER = 5;
        const int STDOUT_FILTER = 6;
        const int STDERR_FILTER = 7;
        const int ARGUMENTS = 8;

        BuildTool* build_tool = (BuildTool*) luaxx_check( lua_state, BUILD_TOOL, BUILD_TOOL_TYPE );

        const char* command = luaL_checkstring( lua_state, COMMAND );
        const char* command_line = luaL_checkstring( lua_state, COMMAND_LINE );
        unique_ptr<process::Environment> environment;
        if ( !lua_isnoneornil(lua_state, ENVIRONMENT) )
        {
            if ( !lua_istable(lua_state, ENVIRONMENT) )
            {
                lua_pushstring( lua_state, "Expected an environment table or nil as 3rd parameter" );
                return lua_error( lua_state );
            }
            
            environment.reset( new process::Environment );
            lua_pushnil( lua_state );
            while ( lua_next(lua_state, ENVIRONMENT) )
            {
                if ( lua_isstring(lua_state, -2) )
                {
                    const char* key = lua_tostring( lua_state, -2 );
                    const char* value = lua_tostring( lua_state, -1 );
                    environment->append( key, value );
                }
                lua_pop( lua_state, 1 );
            }
        }

        unique_ptr<Filter> dependencies_filter;
        if ( !lua_isnoneornil(lua_state, DEPENDENCIES_FILTER) )
        {
            if ( !lua_isfunction(lua_state, DEPENDENCIES_FILTER) && !lua_istable(lua_state, DEPENDENCIES_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 4th parameter (dependencies filter)" );
                return lua_error( lua_state );
            }
            dependencies_filter.reset( new Filter(build_tool->lua_state(), lua_state, DEPENDENCIES_FILTER) );
        }

        unique_ptr<Filter> stdout_filter;
        if ( !lua_isnoneornil(lua_state, STDOUT_FILTER) )
        {
            if ( !lua_isfunction(lua_state, STDOUT_FILTER) && !lua_istable(lua_state, STDOUT_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 5th parameter (stdout filter)" );
                return lua_error( lua_state );
            }
            stdout_filter.reset( new Filter(build_tool->lua_state(), lua_state, STDOUT_FILTER) );
        }

        unique_ptr<Filter> stderr_filter;
        if ( !lua_isnoneornil(lua_state, STDERR_FILTER) )
        {
            if ( !lua_isfunction(lua_state, STDERR_FILTER) && !lua_istable(lua_state, STDERR_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 6th parameter (stderr filter)" );
                return lua_error( lua_state );
            }
            stderr_filter.reset( new Filter(build_tool->lua_state(), lua_state, STDERR_FILTER) );
        }

        unique_ptr<Arguments> arguments;
        if ( lua_gettop(lua_state) >= ARGUMENTS )
        {
            arguments.reset( new Arguments(build_tool->lua_state(), lua_state, ARGUMENTS, lua_gettop(lua_state) + 1) );
        }

        build_tool->scheduler()->execute( 
            string(command), 
            string(command_line), 
            environment.release(), 
            dependencies_filter.release(), 
            stdout_filter.release(), 
            stderr_filter.release(), 
            arguments.release(), 
            build_tool->context() 
        );

        return lua_yield( lua_state, 0 );
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

int LuaBuildTool::print( lua_State* lua_state )
{
    const int BUILD_TOOL = 1;
    const int TEXT = 2;
    BuildTool* build_tool = (BuildTool*) luaxx_check( lua_state, BUILD_TOOL, BUILD_TOOL_TYPE );
    build_tool->output( luaL_checkstring(lua_state, TEXT) );
    return 0;
}