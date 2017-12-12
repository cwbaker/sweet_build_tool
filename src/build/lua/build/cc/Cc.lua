
local function call( cc, definition )
    build.merge( cc, definition );
    local settings = cc.settings;
    local architecture = cc.architecture;
    for _, value in ipairs(definition) do
        local source = build.SourceFile( value );
        source:set_required_to_exist( true );
        source.unit = cc;
        source.settings = settings;

        local object = build.File( ("%s/%s/%s/%s"):format(obj_directory(cc), architecture, relative(source:branch()), obj_name(value)) );
        object.source = value;
        object:add_dependency( source );
        object:add_dependency( Directory(object:branch()) );
        cc:add_dependency( object );
    end
end

local function build_( cc_ )
    if cc_:outdated() then
        cc( cc_ );
    end
end

local function create_target_prototype( id, language )
    local target_prototype = build.TargetPrototype( id );
    local function create( settings, architecture )
        local cc = build.Target( anonymous(), target_prototype );
        cc.settings = settings;
        cc.architecture = architecture or settings.default_architecture;
        cc.language = language;
        return cc;
    end
    
    target_prototype.create = create;
    target_prototype.call = call;
    target_prototype.build = build_;
    return target_prototype;
end

create_target_prototype( "Cc", "c" );
create_target_prototype( "Cxx", "c++" );
create_target_prototype( "ObjC", "objective-c" );
create_target_prototype( "ObjCxx", "objective-c++" );
