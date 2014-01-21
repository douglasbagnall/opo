Opo:  a small Video Whale
=========================

Opo is designed to show synchronised full screen video on projectors
connected to multiple video cards.  It can also do other things, like
split video across a number of non-full-screen windows, but that is
sort of incidental.

Dependencies
============

Gstreamer 0.10
GTK/GDK 2.22

You need the dev packages.

To run across multiple screens you need multiple video outputs; for
more than 2 screens this means having more than one video card, which
in turn means having a motherboard capable of handling multiple cards.
Not all combinations of video cards will work.

Compiling
=========

Try ``make``.

Usage
=====

::

   #show a test picture split over 3 windows
   ./opo -F 2 -s 3

   #split a picture full-screen across 4 monitors connected to 2 cards
   ./opo -c picture.avi -w 1024 -h 768 -x 2 -f -s 4

   #see what else you can try
   ./opo --help


Importing video
===============

Opo-launcher provides a means to stitch together several single-window
video sequences windows into an opo-ready multi-window video.

The examples directory has scripts showing how this process can be
controlled in greater detail.

Name and history
================

In 2002 or before, Zeeshan Ali Khattak and others made a video wall
using Gstreamer and Xinerama.  They called it `Video Whale`_ (also
described here_).  To
run the 4x4 array of monitors, they had 4 computers with 4 video cards
in each, and a fifth computer that fed video to the others over the
network.  Opo was inspired by that project.

`Ngā Hau E Whā`_ is an artwork developed by Leilani Kake for the 2011
Auckland Arts Festival, to be shown at Fresh Gallery Otara[3].  It
requires four perfectly synchronised video projections.

Opo_ was a famous New Zealand dolphin.

This software was written for Leilani Kake's work, and Opo is a
conveniently short and available name for a small New Zealand Video
Whale.

.. _`Video Whale`: http://gstreamer.freedesktop.org/apps/videowhale.html
.. _here: http://www.linux-1u.net/X11/Quad/gstreamer.net/video-wall-howto.html
.. _`Ngā Hau E Whā`: http://ngahauewha.wordpress.com/
.. _Opo: http://en.wikipedia.org/wiki/Opo_the_Dolphin


Copyright and License
=====================

Copyright 2011 Douglas Bagnall

Portions of opo.c were originally derived from examples provided by
the Gstreamer developers.

Provided under the terms of the Gnu General Public License Version 3
(see the file COPYING or http://www.gnu.org/licenses/).

BUGS / TODO
===========

* Sometimes opo fails to show all its windows.  This seems to be a
  race condition.

* opo ought to be able to auto-detect size.

* opo-launcher could use the stitching process's progress report

* start_stitching_process, and others could shift to a module, and
  testable bits could be dragged out of Launcher

* Various encoding and muxing settings could be fixed (or removed).

* opo could (optionally/ automatically) use vertical stitching rather
  than horizontal (i.e., 1x4 rather than 4x1), or both (2x2). Apart
  from making a 4 x 1024x768 video 1 pixel wider than is allowed for
  mpeg1/2 (which could conceivably be a sweet spot for fast decoding
  off slow media), the 4x1 arrangement could possibly complicate
  cropping and damage cache locality.
