//
// main.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Application.hpp"
#include <assert/assert.hpp>
#include <exception>
#include <stdio.h>
#include <stdlib.h>

using namespace sweet::forge;

int main( int argc, char** argv )
{
    int result = EXIT_FAILURE;

    try
    {
        Application application( argc, argv );
        result = application.get_result();
    }

    catch ( const std::exception& exception )
    {
        fprintf( stderr, "forge: %s.\n", exception.what() );
        result = EXIT_FAILURE;
    }

    catch ( ... )
    {
        fprintf( stderr, "forge: An unexpected error occured.\n" );
        result = EXIT_FAILURE;
    }

    return result;
}
