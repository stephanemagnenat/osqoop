Osqoop
======


Osqoop is a multi-platform open source software oscilloscope based on Qt 4. It connects to various hardware data sources such as the sound input or a dedicated USB board. Osqoop provides real-time signal processing through a plugin architecture.

Compilation
-----------

Osqoop uses cmake (http://cmake.org) and is thus easy to compile. It supports out of source build and should compile on any major desktop operating system. To quickly compile and run Osqoop under Unix, type the following:

    mkdir build && mkdir target
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=../target ..
    make
    make install
    cd ../target && bin/osqoop

Usage
-----

Osqoop comes with an end-user manual. Look at in the doc/manual directory.

On Linux, the sound card datasource uses OSS, which is outdated now, as ALSA is the standard. However, ALSA provides a OSS-wrapper, called aoss. If you launch Osqoop thrigh it:

    aoss osqoop

the sound card datasource should work.

Hacking
-------

Osqoop is fully documented through Doxygen. Run `doxygen` to build the developer documentation in `doc/dev`.


Credits
-------

Osqoop was initially developed by Stéphane Magnenat at the Laboratory of Digital Systems at Engineering School of Geneva for the TSE Project. Since then various people have contributed plugins. See authors file for details about contributors.

License
-------

Osqoop is open source under GPL v2. See [license](license) and [authors](authors) files.
