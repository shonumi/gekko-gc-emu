# Janitorial/Beginner Tasks #

Simple projects which can be used to get used to the Gekko source:
  * Clean up old code to conform with the project style guide
  * Reimplement .GCM.DMP (source exists as Windows-specific File IO, needs to be converted to standard file IO and tested)
  * Test old game-specific INI patches and port required ones to the new XML system
  * Implement cheat code system
  * Search for TODO and fix :D A lot of this is stuff that was commented out because its not portable, but relatively easy to port to Unix
  * Implement XML saving for configuration

# Intermediate Tasks #

  * Fix issues with Recompiler portability on Linux/OSX
  * Fix issues with Recompiler random crashes (where the interpreter otherwise works, such as with SSBM and ZWW)
  * More complete implementation of CP
  * More complete implementation of PE
  * Revamp HLE (better recognition of functions, more HLE'd code, more optimizations, cleanup code)
  * Fix memory card bug – Memory cards are mostly implemented, but there is currently a corruption bug that prevents them from being formatted.
  * SSE2 optimizations wherever appropriate (e.g. texture decoding, vertex loading, ...)
  * Enhance debugger capabilities (function detection, gfx debugger, ...)

# WIP Tasks #

  * Implement additional SI modes (_Chrono_)
  * Begin implementation of VideoBase class for the new OpenGL renderer (_ShizZy_)
  * Write a Qt4 GUI for testers (load games, generate XML configs, etc) (_neobrain_)
  * Write a Qt4 debugger GUI for developers (_neobrain_)
  * More DSP emulation (HLE or LLE) (_shonumi_)
  * Clean up and unify volume loading code, eventually add support for ciso images (Dolphin has a really nice implementation of this in its DiscIO code, we should have something like this as well) (_shonumi_)

# Completed Tasks #

  * Update GCM/loaders to use standard file IO (_ShizZy_)
  * Fix issues with Recompiler configuration on Windows (_ShizZy_)
  * CMake build system for Linux/OSX (_neobrain_)
  * Write an SDL joypad implementation of the InputBase class (_shonumi_)
  * Figure out how to use SDL input with a Qt-managed render window (_shonumi_)