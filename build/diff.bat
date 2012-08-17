set BC=%GS%\beyondcompare\BCompare.exe

start %BC% bdlog_2005.sln bdlog_2008.sln
start %BC% bdlog_2005.vcproj bdlog_2008.vcproj
start %BC% testbdlog_2005.vcproj testbdlog_2008.vcproj
start %BC% bdlogview_2005.vcproj bdlogview_2008.vcproj

