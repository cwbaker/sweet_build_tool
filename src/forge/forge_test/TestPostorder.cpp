
#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/build_tool/BuildToolEventSink.hpp>
#include <UnitTest++/UnitTest++.h>

using namespace sweet::build_tool;

SUITE( TestPostorder )
{
    TEST_FIXTURE( ErrorChecker, error_from_lua_in_postorder_visit_is_reported_and_handled )
    {
        const char* script = 
            "local ErrorInPostorderVisit = build:target_prototype( 'ErrorInPostorderVisit' ); \n"
            "local error_in_postorder_visit = build:target( 'error_in_postorder_visit', ErrorInPostorderVisit ); \n"
            "build:postorder( function(target) error('Error in postorder visit') end, error_in_postorder_visit ); \n"
        ;        
        test( script );
        CHECK_EQUAL( "[string \"local ErrorInPostorderVisit = build:target_pr...\"]:3: Error in postorder visit", messages[0] );
        CHECK_EQUAL( "Postorder visit of 'error_in_postorder_visit' failed", messages[1] );
        CHECK( errors == 2 );
    }
    
    TEST_FIXTURE( ErrorChecker, unexpected_error_from_lua_in_postorder_visit_is_reported_and_handled )
    {
        const char* script = 
            "local UnexpectedErrorInPostorderVisit = build:target_prototype( 'UnexpectedErrorInPostorderVisit' ); \n"
            "local unexpected_error_in_postorder_visit = build:target( 'unexpected_error_in_postorder_visit', UnexpectedErrorInPostorderVisit ); \n"
            "build:postorder( function(target) foo.bar = 2; end, unexpected_error_in_postorder_visit ); \n"
        ;        
        test( script );
        if ( messages.size() == 2 )
        {
            CHECK_EQUAL( "[string \"local UnexpectedErrorInPostorderVisit = build...\"]:3: attempt to index a nil value (global 'foo')", messages[0] );
            CHECK_EQUAL( "Postorder visit of 'unexpected_error_in_postorder_visit' failed", messages[1] );
        }
        CHECK( errors == 2 );
    }
    
    TEST_FIXTURE( ErrorChecker, recursive_postorder_is_reported_and_handled )
    {
        const char* script = 
            "local RecursivePostorderError = build:target_prototype( 'RecursivePostorderError' ); \n"
            "local recursive_postorder_error = build:target( 'recursive_postorder_error', RecursivePostorderError ); \n"
            "build:postorder( function(target) build:postorder(function(target) end, recursive_postorder_error) end, recursive_postorder_error ); \n"
        ;
        test( script );
        if ( messages.size() == 2 )
        {
            CHECK_EQUAL( "[string \"local RecursivePostorderError = build:target_...\"]:3: Postorder called from within another bind or postorder traversal", messages[0] );
            CHECK_EQUAL( "Postorder visit of 'recursive_postorder_error' failed", messages[1] );
        }
        CHECK( errors == 2 );
    }

    TEST_FIXTURE( ErrorChecker, recursive_postorder_during_postorder_is_reported_and_handled )
    {
        const char* script = 
            "local RecursivePostorderError = build:target_prototype( 'RecursivePostorderError' ); \n"
            "local recursive_postorder_error = build:target( 'recursive_postorder_error', RecursivePostorderError ); \n"
            "build:postorder( function(target) build:postorder(function(target) end, recursive_postorder_error) end, recursive_postorder_error ); \n"
        ;
        test( script );
        if ( messages.size() == 2 )
        {
            CHECK_EQUAL( "[string \"local RecursivePostorderError = build:target_...\"]:3: Postorder called from within another bind or postorder traversal", messages[0] );
            CHECK_EQUAL( "Postorder visit of 'recursive_postorder_error' failed", messages[1] );
        }
        CHECK( errors == 2 );
    }
}