bgsubtract0r
============

Frei0r plugin to perform background subtraction on a still video.

This plugin operates on still videos, i.e. the camera is not moving, and its aim
is to remove the background image, keeping only the moving parts. It is thus a
way to have a green screen without a green screen. The first image is taken as
reference for the background, so the video should start without the moving
stuff, who should come in afterwards.

A few settings can be set such as:
- the color of the green screen to generate
- the threshold in order to decide whether a pixel is the same as in the
  reference image
- whether to remove "noise" (i.e. isolated pixels)

Have fun and don't hesitate to use the bugtracker!

Samuel Mimram
