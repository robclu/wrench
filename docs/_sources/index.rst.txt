
Welcome to Wrench's documentation!
======================================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   api/wrench_api_root.rst

.. role:: bash(code)
   :language: bash

.. role:: cmake(code)
   :language: cmake

This is the official documentation for Wrench. Wrench is a collection of
general-purpose utilities written in C++. It is simply documented
implementations of things that I have used in projects. It's free to either use
in its entirety, or take what you want from it. I hope it's somewhat useful to
someone.

Versions and Release
====================

This project is always on going, and master will always be a work in progress.
When stable releases are available, they will be put into their own branches.

OS Support
==========

Wrench has been developed on OSX and linux, and works there. It should work on
Windows, but is not tested. Features which are only available on some OS's are
documented.

Installing
==========

Currently this is a header-only library, and can either be used as a
subdirectory in an existing CMake project by :cmake:`add_subdirectory(wrench)`,
or by installing as follows (from the root directory):
::

   mkdir build && cd build
   cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX={PREFIX} ..
   make -j4
   make install

which will install wrench to :bash:`PREFIX`.

.. note::
   You will need to add :bash:`PREFIX` to :bash:`CMAKE_PREFIX_PATH` if
   :cmake:`find_package(wrench)` doesn't work after installation.
