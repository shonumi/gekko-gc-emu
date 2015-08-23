# Building Qt4 (Windows) #
- download the latest [Qt SDK](http://qt.nokia.com/downloads/sdk-windows-cpp) (1 GB, less when using the online installer) and install it. The only required component is _Desktop Qt 4.8.0_, but you'll also need _Qt Designer_ if you want to modify Gekko's user interface. Furthermore, _Qt Examples_, _Qt SDK Documentation_ and _Qt Assistant_ are quite useful for developing Qt applications.

- set your QTDIR environment variable to the directory which contains the bin, include and lib folders (e.g. C:\QtSDK\Desktop\Qt\4.8.0\msvc2010)

- for running Gekko builds, you'll need to have QtCore4.dll and QtGui4.dll in either the application directory or in the system PATH.


# Building Qt4 (Linux) #
- the Qt4 GUI is enabled by default when using CMake if you have the Qt development files installed

- the option DISABLE\_QT4 can be used to disable building the GUI. Use cmake -i to enable it.


# Best Practices #
- use the extension .hxx for header files that need to be moc-ed and .ui for user interfaces created with Qt Designer. That way, build rules for these files will get applied automatically.

- always use the _tr_ function for strings which need to be translated in case we ever decide to support translations

- use Qt Designer for creating user interfaces (dialogs, dock widgets, etc.) wherever possible. I tried creating a complex application before using plain C++-code and I ended up wasting lots of time while gaining virtually no advantages.

- when adding include directories to the VS project files, also make sure to add them to the custom MOC build rule's C++ command line options. Otherwise compilation will fail when trying to compile the moc output. Example: You added _$(SolutionDir)\src\common\src_ to the additional include directories. You also should add _/I"$(SolutionDir)\src\common\src"_ to the C++ command line options of the _Qt Meta Object Compiler_ rule in the gekko\_qt project configuration properties.