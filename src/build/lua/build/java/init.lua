
java = {};

function java.configure( settings )
    function autodetect_jdk_directory()
        if build:operating_system() == "windows" then
            return "C:/Program Files/Java/jdk1.6.0_39";
        else
            return "/Library/Java/JavaVirtualMachines/jdk1.7.0_79.jdk/Contents/Home";
        end
    end

    local local_settings = build.local_settings;
    if not local_settings.java then
        local_settings.updated = true;
        local_settings.java = {
            jdk_directory = autodetect_jdk_directory();
        };
    end
end

function java.add_jar_dependencies( jar, jars )
    if jars and platform ~= "" then
        for _, value in ipairs(jars) do
            jar:add_dependency( build:target(build:root(value)) );
        end
    end
end

require "build.java.Jar";
require "build.java.Java";

build:register_module( java );