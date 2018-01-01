
local Java = build.TargetPrototype( "Java" );

function Java.create( settings, definition )
    local java = build.Target( build.anonymous(), Java, definition );
    java.settings = settings;
    java:set_filename( ("%s/Java.%s.timestamp"):format(build.classes_directory(java), java:id()) );
    java:add_ordering_dependency( build.Directory(build.classes_directory(java)) );

    build.pushd( java.sourcepath or "." );
    for _, value in ipairs(definition) do
        if type(value) == "string" then
            local source = build.file( value );
            source:set_required_to_exist( true );
            java:add_dependency( source );
        elseif type(value) == "table" then
            java:add_dependency( value );
        end
    end
    build.popd();

    build.add_jar_dependencies( java, java.settings.jars );
    build.add_jar_dependencies( java, definition.jars );

    return java;
end

function Java.build( java )
    if java:outdated() then
        local jars = {};
        local source_files = {};
        table.insert( jars, ("%s/platforms/%s/android.jar"):format(settings.android.sdk_directory, settings.android.sdk_platform) );
        for _, dependency in java:dependencies() do 
            local prototype = dependency:prototype();
            if prototype == nil then
                table.insert( source_files, build.relative(dependency:filename()) );
            elseif prototype == build.Jar then
                table.insert( jars, build.relative(dependency:filename()) );
            end
        end

        if #source_files > 0 then
            local settings = java.settings;
            local javac = ("%s/bin/javac"):format( settings.android.jdk_directory );
            local output = build.classes_directory( java );
            local classpath = output;

            local sourcepaths = {};
            if java.sourcepaths then
                for _, path in ipairs(java.sourcepaths) do 
                    table.insert( sourcepaths, path );
                end
            end
            if settings.sourcepaths then 
                for _, path in ipairs(java.settings.sourcepaths) do 
                    table.insert( sourcepaths, path );
                end
            end
            table.insert( sourcepaths, java.sourcepath or "." );
            table.insert( sourcepaths, build.gen_directory(java) );

            if java.third_party_jars then 
                for _, jar in ipairs(java.third_party_jars) do 
                    table.insert( jars, jar );
                end
            end
            if settings.third_party_jars then 
                for _, jar in ipairs(settings.third_party_jars) do 
                    table.insert( jars, jar );
                end
            end

            build.system( javac, ('javac -Xlint:unchecked -d "%s" -sourcepath "%s" -classpath "%s" -target 1.7 -bootclasspath "%s" -encoding UTF-8 -g -source 1.7 %s'):format(output, table.concat(sourcepaths, ":"), classpath, table.concat(jars, ":"), table.concat(source_files, " ")) );

            local timestamp_file = io.open( java:filename(), "wb" );
            assertf( timestamp_file, "Opening '%s' to write generated text failed", java:filename() );
            timestamp_file:write( ("# Timestamp for '%s'"):format(java:path()) );
            timestamp_file:close();
            timestamp_file = nil;
        end
    end
end

function Java.clean( java )
    build.rm( java:filename() );
    build.rmdir( build.classes_directory(java) )
end

android.Java = Java;