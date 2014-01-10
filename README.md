CDex
====

Very well known and very good CD-Ripper running on Windows XP and later.

This is a cloned Repo from [cdexos.sourceforge.net](cdexos.sourceforge.net).

The current release on sf.net 1.70 b4 has a nasty bug during submitting data to cddb. The bug was caused by non-standard-chars somewhere in the received data, sometimes this problematic data is not displayed within the dialog. In my case the media-type was "Hörbuch" the german word for audiobook. Thanks to wech71 for fixing the problem.

We simply took the code, moved it to VS2013 and fixed a few small bugs.

et voilá: Here's release 1.71: [Install\cdex_171_enu.zip](https://github.com/elha/CDex/raw/master/Install/cdex_171_enu.zip)

I don't want to offend the people who developed such a great piece of software. Thanks for you work! I just wanted to help out and fix this small thing.

elha