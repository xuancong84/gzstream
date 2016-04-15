# gzstream: Automatic/smart gzip stream wrapper for C++ (C++11 compatible)

Features:
- igzfstream and ogzfstream are automatic I/O streams
- automatic means: if a filename ends with .gz, it will compress/decompress the file; if the filename is -, it will redirect to standard IO
- igzstream and ogzstream are non-automatic gzip I/O streams



To build for use:
- add gzstream.cpp into your project
- include the header file gzstream.h in your C++ program
- when you link, remember to add -lz at the end


To build for testing:
g++ gzstream.cpp -lz -DDEBUG\_GZIP && ./a.out




