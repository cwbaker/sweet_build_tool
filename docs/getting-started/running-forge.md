---
layout: page
title: Running Forge
parent: Getting Started
nav_order: 1
---

## Usage

~~~sh
Usage: forge [options] [variable=value] [command] ...
Options:
  -h, --help         Print this message and exit.
  -v, --version      Print the version and exit.
  -r, --root         Set the root directory.
  -f, --file         Set the name of the root build script.
  -s, --stack-trace  Enable stack traces in error messages.
Variables:
  goal               Target to build (relative to working directory).
  variant            Variant built (debug, release, shipping).
Commands:
  build              Build outdated targets.
  clean              Clean all targets.
  reconfigure        Regenerate per-machine configuration settings.
  dependencies       Print targets by dependency hierarchy.
  namespace          Print targets by namespace hierarchy.
~~~

When run from a directory within the source tree of the project being built Forge searches up from the current working directory to the root of the file system looking for files named *forge.lua*.  The *forge.lua* file found in the highest directory in the hierarchy is the root build script executed to define and run the build.  The directory that contains the root build script becomes the root directory of the project.

The initial working directory is the directory that Forge is run from.  By default the target named *all* in this directory is built.  Building in the root directory of the project typically builds all useful outputs for a project.  Building in sub-directories of the project typically builds targets defined in that directory only.

Pass commands (e.g. *clean*, *build*, *dependencies*, etc) to determine what the build does and in what order.  The default, when no other command is passed, is *build* which typically brings all files up to date by building them.

Multiple commands passed on the same command line are executed in order.  The dependency graph is restored between commands so passing multiple commands to one invocation is functionally the same as passing the same commands to separate invocations.  Duplicate commands are executed multiple times.

Assign values to variables (e.g. *variant={debug, release, shipping}*) on the command line to configure the build.  All assignments are made to global variables in Lua before the root build script and any actions are executed.  Typically this is used to configure variant, target to build, and/or install location.

Later assignments override earlier ones in the case of duplicate variables.  However because all assignments are made before any commands are executed interleaving assignments and commands is not generally useful.

Build useful outputs by running from the project's root directory:

~~~bash
$ forge
~~~

Remove generated files with the *clean* command:

~~~bash
$ forge clean
~~~

Rebuild by running the *clean* and *build* commands in the same invocation:

~~~bash
$ forge clean build
~~~

Build the *release* variant by setting `variant=release`:

~~~bash
$ forge variant=release
~~~

Clean the *release* variant by setting `variant=release` with the *clean* command:

~~~bash
$ forge variant=release clean
~~~

Regenerate settings for the local machine by running *reconfigure*:

~~~bash
$ forge reconfigure
~~~

List the dependency graph for the *release* variant:

~~~bash
$ forge variant=release dependencies
~~~