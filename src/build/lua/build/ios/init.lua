
ios = {};

function ios.sdkroot_by_target_and_platform( target, platform )
    local sdkroot = target.settings.sdkroot or "iphoneos";
    if platform == "ios_simulator" then 
        sdkroot = string.gsub( sdkroot, "iphoneos", "iphonesimulator" );
    end
    return sdkroot;
end

function ios.configure( settings )
    local function autodetect_iphoneos_sdk_version()
        local sdk_version = "";
        local sdk_build_version = "";

        local xcodebuild = "/usr/bin/xcodebuild";
        local arguments = "xcodebuild -sdk iphoneos -version";
        local result = execute( xcodebuild, arguments, nil, function(line)
            local key, value = line:match( "(%w+): ([^\n]+)" );
            if key and value then 
                if key == "ProductBuildVersion" then 
                    sdk_build_version = value;
                elseif key == "SDKVersion" then
                    sdk_version = value;
                end
            end
        end );
        assert( result == 0, "Running xcodebuild to extract SDK name and version failed" );

        return sdk_version, sdk_build_version;
    end

    local function autodetect_xcode_version()
        local xcode_version = "";
        local xcode_build_version = "";

        local xcodebuild = "/usr/bin/xcodebuild";
        local arguments = "xcodebuild -version";
        local result = execute( xcodebuild, arguments, nil, function(line)
            local major, minor = line:match( "Xcode (%d+)%.(%d+)" );
            if major and minor then 
                xcode_version = ("%02d%02d"):format( tonumber(major), tonumber(minor) );
            end

            local build_version = line:match( "Build version (%w+)" )
            if build_version then
                xcode_build_version = build_version;
            end
        end );
        assert( result == 0, "Running xcodebuild to extract Xcode version failed" );
        
        return xcode_version, xcode_build_version;
    end

    local function autodetect_macosx_version()
        local os_version = "";

        local sw_vers = "/usr/bin/sw_vers";
        local arguments = "sw_vers -buildVersion";
        local result = execute( sw_vers, arguments, nil, function(line)
            local version = line:match( "%w+" );
            if version then 
                os_version = version;
            end
        end );
        assert( result == 0, "Running sw_vers to extract operating system version failed" );

        return os_version;
    end

    if operating_system() == "macosx" then
        local local_settings = build.local_settings;
        if not local_settings.ios then
            local sdk_version, sdk_build_version = autodetect_iphoneos_sdk_version();
            local xcode_version, xcode_build_version = autodetect_xcode_version();
            local os_version = autodetect_macosx_version();
            local_settings.updated = true;
            local_settings.ios = {
                xcrun = "/usr/bin/xcrun";
                signing_identity = "iPhone Developer";
                codesign_allocate = "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/codesign_allocate";
                codesign = "/usr/bin/codesign";
                plutil = "/usr/bin/plutil";
                sdk_name = "iphoneos";
                sdk_version = sdk_version;
                sdk_build_version = sdk_build_version;
                xcode_version = xcode_version;
                xcode_build_version = xcode_build_version;
                os_version = os_version;
            };
        end
    end
end;

function ios.initialize( settings )
    if build.platform_matches("ios.*") then
        cc = ios.cc;
        objc = ios.objc;
        build_library = ios.build_library;
        clean_library = ios.clean_library;
        build_executable = ios.build_executable;
        clean_executable = ios.clean_executable;
        lipo_executable = ios.lipo_executable;
        obj_directory = ios.obj_directory;
        cc_name = ios.cc_name;
        cxx_name = ios.cxx_name;
        obj_name = ios.obj_name;
        lib_name = ios.lib_name;
        dll_name = ios.dll_name;
        exe_name = ios.exe_name;
        module_name = ios.module_name;
    end
end;

function ios.cc( target )
    local flags = {
        '-DBUILD_OS_IOS'
    };
    clang.append_defines( target, flags );
    clang.append_include_directories( target, flags );
    clang.append_compile_flags( target, flags );

    local iphoneos_deployment_target = target.settings.iphoneos_deployment_target;
    if iphoneos_deployment_target then 
        table.insert( flags, ("-miphoneos-version-min=%s"):format(iphoneos_deployment_target) );
    end

    local sdkroot = ios.sdkroot_by_target_and_platform( target, platform );
    local ccflags = table.concat( flags, " " );
    local xcrun = target.settings.ios.xcrun;

    for dependency in target:dependencies() do
        if dependency:outdated() then
            print( leaf(dependency.source) );
            build.system( 
                xcrun, 
                ('xcrun --sdk %s clang %s -o "%s" "%s"'):format(sdkroot, ccflags, dependency:filename(), absolute(dependency.source)),
                nil, 
                build.dependencies_filter(dependency) 
            );
        end
    end
end;

function ios.build_library( target )
    local flags = {
        "-static"
    };

    pushd( ("%s/%s"):format(obj_directory(target), target.architecture) );
    local objects =  {};
    for dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype == build.Cc or prototype == build.Cxx or prototype == build.ObjC or prototype == build.ObjCxx then
            for object in dependency:dependencies() do
                table.insert( objects, relative(object:filename()) );
            end
        end
    end
    
    if #objects > 0 then
        local sdkroot = ios.sdkroot_by_target_and_platform( target, platform );
        local arflags = table.concat( flags, " " );
        local arobjects = table.concat( objects, [[" "]] );
        local xcrun = target.settings.ios.xcrun;

        print( leaf(target:filename()) );
        build.system( xcrun, ('xcrun --sdk %s libtool %s -o "%s" "%s"'):format(sdkroot, arflags, native(target:filename()), arobjects) );
    end
    popd();
end;

function ios.clean_library( target )
    rm( target:filename() );
    rmdir( obj_directory(target) );
end;

function ios.build_executable( target )
    local flags = {};
    clang.append_link_flags( target, flags );
    table.insert( flags, "-ObjC" );
    table.insert( flags, "-all_load" );

    local iphoneos_deployment_target = target.settings.iphoneos_deployment_target;
    if iphoneos_deployment_target then 
        if platform == "ios" then 
            table.insert( flags, ("-mios-version-min=%s"):format(iphoneos_deployment_target) );
        elseif platform == "ios_simulator" then
            table.insert( flags, ("-mios-simulator-version-min=%s"):format(iphoneos_deployment_target) );
        end
    end

    clang.append_library_directories( target, flags );

    local objects = {};
    local libraries = {};

    pushd( ("%s/%s"):format(obj_directory(target), target.architecture) );
    for dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype == build.Cc or prototype == build.Cxx or prototype == build.ObjC or prototype == build.ObjCxx then
            for object in dependency:dependencies() do
                table.insert( objects, relative(object:filename()) );
            end
        elseif prototype == build.StaticLibrary or prototype == build.DynamicLibrary then
            table.insert( libraries, ("-l%s"):format(dependency:id()) );
        end
    end

    clang.append_link_libraries( target, libraries );

    if #objects > 0 then
        local sdkroot = ios.sdkroot_by_target_and_platform( target, platform );
        local ldflags = table.concat( flags, " " );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, " " );
        local xcrun = target.settings.ios.xcrun;

        print( leaf(target:filename()) );
        build.system( xcrun, ('xcrun --sdk %s clang++ %s "%s" %s'):format(sdkroot, ldflags, ldobjects, ldlibs) );
    end
    popd();
end

function ios.clean_executable( target )
    rm( target:filename() );
    rmdir( obj_directory(target) );
end

function ios.lipo_executable( target )
    local executables = {};
    for executable in target:dependencies() do 
        table.insert( executables, executable:filename() );
    end
    print( leaf(target:filename()) );
    local sdk = ios.sdkroot_by_target_and_platform( target, platform );
    executables = table.concat( executables, [[" "]] );
    local xcrun = target.settings.ios.xcrun;
    build.system( xcrun, ('xcrun --sdk %s lipo -create -output "%s" "%s"'):format(sdk, target:filename(), executables) );
end

-- Deploy the fist iOS .app bundle found in the dependencies of the current
-- working directory.
function ios.deploy( directory )
    local ios_deploy = build.settings.ios.ios_deploy;
    if ios_deploy then 
        local directory = directory or find_target( initial() );
        local app = nil;
        for dependency in directory:dependencies() do
            if dependency:prototype() == ios.App then 
                app = dependency;
                break;
            end
        end
        assertf( app, "No ios.App target found as a dependency of '%s'", directory:path() );
        assertf( is_file(ios_deploy), "No 'ios-deploy' executable found at '%s'", ios_deploy );
        build.system( ios_deploy, ('ios-deploy --timeout 1 --bundle "%s"'):format(app:filename()) );
    else
        printf( ios_deploy, "No 'ios-deploy' executable specified in settings" );
    end
end

function ios.obj_directory( target )
    return ("%s/%s_%s/%s"):format( target.settings.obj, platform, variant, relative(target:working_directory():path(), root()) );
end

function ios.cc_name( name )
    return ("%s.c"):format( basename(name) );
end

function ios.cxx_name( name )
    return ("%s.cpp"):format( basename(name) );
end

function ios.obj_name( name, architecture )
    return ("%s.o"):format( basename(name) );
end

function ios.lib_name( name, architecture )
    return ("lib%s_%s.a"):format( name, architecture );
end

function ios.dll_name( name )
    return ("%s.dylib"):format( name );
end

function ios.exe_name( name, architecture )
    return ("%s_%s"):format( name, architecture );
end

function ios.module_name( name, architecture )
    return ("%s_%s"):format( name, architecture );
end

require "build.ios.App";

build.register_module( ios );