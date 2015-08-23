# Welcome to the Gekko Project Site #
Gekko is an experimental Nintendo GameCube emulator started in 2006 by ShizZy and Lightning. It was originally written for 32-bit Windows and was capable of booting many commercial games. It now has early 64-bit and Linux support, and now focuses on being very portable (with only pure C/C++) while still achieving reasonable speed.

## Wasn't Gekko dead?!?! ##
After about 3 years of inactivity, ShizZy started a new build of Gekko in early 2012. The goal of the project is primarily to revive Gekko and its community, but also to help bring some life back to the emulation scene :) This build of the project is planned to include a OpenGL 3.3-based video core, XML-based configuration manager, dual-core support, and better cross-platform support. The project is also intended to be a platform to eventually expand to Wii (and future Nintendo system) emulation.

The Gekko while the project website and forums are hosted on http://6bit.net.

Gekko is licensed under the GNU GPL v2.

## Getting started with Gekko Development ##

If you're new to the project, start here!

### Building the Source ###

**Windows**: We're currently developing with Microsoft Visual C++ 2010. All dependencies are checked into the tree and referenced by the project – so that's all you need! Simply open Visual Studio and select "Build All". It's that simple!

**Linux**: See [LinuxBuild](LinuxBuild.md).

**OSX**: We don't currently have someone actively maintaining OSX support for Gekko. But if you'd like to, please let us know!

### Rules ###
  * **DO** code with style! We use a coding style guideline for lots of good reasons (if you want to know them, ask ShizZy on #gekko). Our style guideline is the [Google C++ Style Guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml), with two exceptions:
    1. 80-character width is too restrictive, you can exceed that. Try to keep it to 100 if possible, though.
    1. We use 4-spaces for tabs (not tabs!)
> There is lots of old code that doesn't follow this, but that doesn't mean you can't. If you commit code that is messy and non-consistent, we will be angry at you!
  * **DO NOT** introduce dependencies to the project without good reason! Use the following guidelines:
    1. If you absolutely need to use a new external library, make sure you check it into the /externals folder. Include the source, with buildable project files, and the latest binaries for our build environments.
    1. Dependencies must be portable!  (i.e. Windows, Linux, and OS X supported)
    1. Dependencies must be as light-weight as possible to make potential porting efforts easier in the future.
    1. Check and make sure we don't already have a library in use that does what you are looking for! Currently libraries include SDL (for just about everything), RapidXML, GLEW, and GLFW.
    1. Keep dependencies out of the "core" (exception: SDL). Create an abstraction layer in "common" instead.
    1. Don't introduce GUI dependencies in "common" or "core". The right place is in a frontend project/module.
  * **DO** Comment your code as much as possible! Don't comment stupid obvious things, rather, comment the flow of your code, algorithms, and non-obvious statements. Try to use Doxygen commenting as much as possible.
  * **DO** use the Issues page to submit bug reports, works in progress, features, etc. to let us know what you are working on.
  * **DO** Update this wiki and create new pages as needed to document the emulator, GameCube and Wii emulation, and your work.
  * **DO** Create a new branch if you are doing new experimental work.
  * **DO NOT** Create a personal branch for anything you're currently working on. Use branches for each piece of work instead.
  * **DO NOT** Make any commits unless you provide a helpful message of what the change does and make sure that it has been tested.

### Project Structure ###
  * **common** – Common code that is useful from all other project modules. This includes library wrappers, OS utilities, general routines, types, etc.
  * **core** – This IS the emulator. Any emulation, unless it is has its own separate module, belongs here. Theoretically, this should be able to be pulled out and linked into other projects as a library for GameCube emulation.
  * **gekko** – This is the application entry point (for no GUI). It uses minimal dependencies, and can be used to run games from a command line.
  * **gekko\_qt** - This is the application entry point for the Qt fronted.
  * **input\_common** - This is the interface for creating new user-input plugins (for GameCube controllers, for example).
  * **emu\_win32** – This is the old (no longer supported) Gekko Win32 GUI.

### Configuring the Emulator ###
We use an XML-based system for configuring the emulator. There is a single XML schema, gekko-conf.xsd, that describes this format. Essentially, there are 3 tiers of configuration:
  * **sysconf.xml** – This is the system configuration file, which is used for project defaults.  Change this file in the SVN only if you are adding a new configuration setting, or have a new global configuration change that positively affects everything and is endorsed by the project team. This is loaded FIRST by the emulator.
  * **userconf.xml** – This is the user configuration file, which is used for user preferencs (i.e. how you like to map your controller input). All system configurations are valid here, and will override any system-wide defaults specified in sysconf.xml. Use this file when debugging with new settings.
  * **"Game-ID".xml** – Lastly, Gekko loads a third game-specific XML configuration file from the /user/games directory (if one exists). This also allows all valid XML, and overrides anything from both sysconf.xml and userconf.xml. The purpose of these configuration files are for game-specific settings required to make individual games boot (e.g. patches, HLE, etc.), or cheats.

That's all there is to it. You can use [Xerces](http://xerces.apache.org/xerces-c/) to validate your XML configurations against the schema if you wish (the emulator doesn't by default).

### I've done everything you said, what's next? ###
Talk to a project admin on the forums, IRC (#gekko on EFnet), or by [email](http://6bit.net/contact-us/), and they may add you to the Google project.