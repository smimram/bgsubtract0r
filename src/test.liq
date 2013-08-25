#!/usr/bin/liquidsoap

set("log.level", 4)
set("log.file", false)
# set("frame.video.width",640)
# set("frame.video.height",480)

bg = single("test.wmv")
bg = drop_audio(bg)

s = single("test.webm")
s = video.frei0r.bgsubtract0r(threshold=0.05,blur=3.,s)
# s = video.transparent(color=0x00ff00,s)
s = add([bg,s])

output.sdl(s)
