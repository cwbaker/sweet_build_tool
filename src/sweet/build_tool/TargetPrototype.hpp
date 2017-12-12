//
// BuildTool.hpp
// Copyright (c) 2007 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_BUILD_TOOL_TARGET_PROTOTYPE_HPP_INCLUDED
#define SWEET_BUILD_TOOL_TARGET_PROTOTYPE_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/pointer/ptr.hpp>
#include <string>

namespace sweet
{

namespace build_tool
{

class BuildTool;

/**
// A prototype for Targets.
*/
class SWEET_BUILD_TOOL_DECLSPEC TargetPrototype
{
    std::string id_; ///< The identifier for this TargetPrototype.
    BuildTool* build_tool_; ///< The BuildTool that this TargetPrototype is part of.

    public:
        TargetPrototype( const std::string& id, BuildTool* build_tool );
        const std::string& get_id() const;
};

}

}

#endif