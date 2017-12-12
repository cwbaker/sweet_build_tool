
local App = build.TargetPrototype( "ios.App" );

function App.create( settings, id )
    local app = build.Target( ("%s.app"):format(id), App );
    app:set_filename( ("%s/%s.app"):format(settings.bin, id) );
    app.settings = settings;
    build.default_target( app );
    build.push_settings {
        bin = app:filename();
        data = app:filename();
    };
    return app;
end

function App.call( app, definition )
    local entitlements = definition.entitlements;
    if entitlements then 
        app.entitlements = ("%s/%s"):format( obj_directory(app), "Entitlements.plist" );
        table.insert( definition, build.Generate(app.entitlements, entitlements) );
    end

    local resource_rules = definition.resource_rules;
    if resource_rules then 
        assertf( is_file(resource_rules), "The resource rules file '%s' does not exist", tostring(resource_rules) );
        app.resource_rules = ("%s/ResourceRules.plist"):format( app:filename() );
        table.insert( definition, build.Copy(app.resource_rules, resource_rules) );
    end

    local working_directory = working_directory();
    for _, dependency in ipairs(definition) do 
        working_directory:remove_dependency( dependency );
        app:add_dependency( dependency );
        dependency.module = app;
    end
end

function App.build( app )
    if app:outdated() then
        local xcrun = app.settings.ios.xcrun;
        if app.settings.generate_dsym_bundle then 
            local executable;
            for dependency in app:dependencies() do 
                if dependency:prototype() == build.Lipo then 
                    executable = dependency:filename();
                    break;
                end
            end
            if executable then 
                build.system( xcrun, ([[xcrun dsymutil -o "%s.dSYM" "%s"]]):format(app:filename(), executable) );
                if app.settings.strip then 
                    build.system( xcrun, ([[xcrun strip "%s"]]):format(executable) );
                end
            end
        end

        local provisioning_profile = _G.provisioning_profile or app.settings.provisioning_profile;
        if provisioning_profile then
            local embedded_provisioning_profile = ("%s/embedded.mobileprovision"):format( app:filename() );
            rm( embedded_provisioning_profile );
            cp( provisioning_profile, embedded_provisioning_profile );
        end

        local command_line = {
            "codesign";
            ('-s "%s"'):format( _G.signing_identity or app.settings.ios.signing_identity );
            "--force";
            "--no-strict";
            "-vv";
            ('"%s"'):format( app:filename() );
        };
        local entitlements = app.entitlements;
        if entitlements then 
            table.insert( command_line, ('--entitlements "%s"'):format(entitlements) );
        end

        local resource_rules = app.resource_rules;
        if resource_rules then 
            table.insert( command_line, ('--resource-rules "%s"'):format(resource_rules) );
        end

        local environment = {
            CODESIGN_ALLOCATE = app.settings.ios.codesign_allocate;
        };
        local codesign = app.settings.ios.codesign;
        build.system( codesign, table.concat(command_line, " "), environment );
    end
end

function App.clean( app )
    rmdir( app:filename() );
end

ios.App = App;