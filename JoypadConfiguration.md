# Introduction #

Gekko now has basic joypad support via SDL. The **sysconf.xml** file holds the configuration settings for the joypad. You'll want to edit the parts enclosed in the JoypadController element.

Each input for the emulated GameCube Controller is represented by a unique number. The SDLJoypads plugin generates this number to map out specific inputs, e.g. button presses or left axis motions. We'll refer to these numbers by calling them **pads**.

# Determining Pads - Buttons #

The SDLJoypads plugin follows some very simple rules when determining pads. All button presses are mapped to pads 100-199 (though the joypad shouldn't have _that_ many buttons...). All joypads reference their buttons internally with numbers. In other words, your joypad has an idea of what it's "first" button is and what it's "second button" is.

Whatever your joypad says is its "first" button gets assigned by SDLJoypads to the pad 100, the second button to pad 101, so on and so forth. If you wanted the "first" button to represent the A button on the emulated GameCube Controller, simply change the value in **sysconf.xml** to read 100.

# Determining Pads - Axes #

With SDLJoypads, all axis input is mapped to pads 200-299. Most joypads also know which axis is moving, assuming it has more than one. For the first axis, left movement is mapped to pad 200, right movement to pad 201, up movement to pad 202, down movement to pad 203, so on and so forth. The next axis starts on the left, with a pad of 204, repeating the process.

# Determining Pads - Hats #

SDLJoypads maps all hat input to pads 300-399. Joypad hats are equivalent to dpads. A vast majority of joypads only have one hat. Very few have two; the Virtual Boy's controller is the only one off the top of my head. Pads are assigned much the same as with axes. For the first hat, left hat input is mapped to pad 300, right hat input is mapped to 301, up hat input is mapped to 302, down hat input is mapped 303, so on and so forth. If you happen to have more than one hat, that next hat starts on the left with a pad of 304. Joypads with more than one hat have not been tested with SDLJoypads, but configuring them should work all the same.

# In Summary #

Manually adding/editing the pads is the only way to configure joypads for now. Hopefully this will change later on. Also note that at the moment SDLJoypads only initializes and configures the first attached joypad. This too will change in the near future.