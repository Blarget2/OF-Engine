--[[! File: library/core/std/gui/init.lua

    About: Author
        q66 <quaker66@gmail.com>

    About: Copyright
        Copyright (c) 2011 OctaForge project

    About: License
        This file is licensed under MIT. See COPYING.txt for more information.

    About: Purpose
        OctaForge standard library loader (GUI system).
]]

log(DEBUG, ":::: Core UI implementation.")
gui = {
    core     = require("std.gui.core"),
    textedit = require("std.gui.textedit")
}
