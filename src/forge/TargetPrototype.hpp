#ifndef SWEET_BUILD_TOOL_TARGET_PROTOTYPE_HPP_INCLUDED
#define SWEET_BUILD_TOOL_TARGET_PROTOTYPE_HPP_INCLUDED

#include <string>

namespace sweet
{

namespace build_tool
{

class BuildTool;

/**
// A prototype for Targets.
*/
class TargetPrototype
{
    std::string id_; ///< The identifier for this TargetPrototype.
    BuildTool* build_tool_; ///< The BuildTool that this TargetPrototype is part of.

    public:
        TargetPrototype( const std::string& id, BuildTool* build_tool );
        ~TargetPrototype();
        const std::string& id() const;
};

}

}

#endif