bdlog
=====

highly extensible c++ log library on windows system
forked from https://svn.baidu.com/app/gensoft/bull/trunk/bdlog:816

bdlog include: 
      bdlog library; 
      bdlogview, a GUI tool for viewing bdlog pipe device output;
      testbdlog, for bdlog functional testing.


dir structure
=============

[bdlog]          files for wrapping log functionality into DLL
[bdlogview]      bdlogview source
[build]          vcprojs for building
[doc]            documents
[include]        [detail]          bdlog implementation
                 bdlog.h           bdlog interface
                 bdlog_util.h      bdlog util interface
                 accutime.h        bdlog sync time reader
                 bdlogprovider.h   bdlog pipe receiver and file reader (bdlogprovider.h)
[testbdlog]      functional test
[thirdsrc]       third party source, only used by bdlogview and testbdlog
[tools]          misc util
