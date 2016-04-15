#pragma once

// standard C++ with new header file names and  namespace
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <zlib.h>


using namespace std;

// ----------------------------------------------------------------------------
// Internal classes to implement gzstream. See below for user classes.
// ----------------------------------------------------------------------------
static string stdio_filename = "-";
bool isGzipFilename(string name);

class gzstreambuf: public streambuf {
public:
	static const int bufferSize = 47 + 256;    // size of data buff
	// totals 512 bytes under g++ for igzstream at the end.

	gzFile file;               // file handle for compressed file
	char buffer[bufferSize]; // data buffer
	char opened;             // open/close state of stream
	int mode;               // I/O mode

	int flush_buffer();

	gzstreambuf() : opened(0) {
		setp(buffer, buffer + (bufferSize - 1));
		setg(buffer + 4,     // beginning of putback area
		buffer + 4,     // read position
		buffer + 4);    // end position
		// ASSERT: both input & output capabilities will not be used together
	}
	gzstreambuf(const char* name, int open_mode) : gzstreambuf(){
		open(name, open_mode);
	}
	int is_open() {
		return opened;
	}
	gzstreambuf* open(const char* name, int open_mode);
	gzstreambuf* close();
	~gzstreambuf() {
		if(is_open())
			close();
	}
	int seek (streamoff off, ios::seekdir way){
		int res = gzseek(file, off, way);
		if(res==-1){
			ostringstream oss;
			oss << (filebuf*)this;
			res = gzseek(file, off, way);
		}
		return res;
	}
	streampos tell (){
		return (streampos)gztell(file);
	}

	virtual int overflow(int c = EOF);
	virtual int underflow();
	virtual int sync();
};

// ----------------------------------------------------------------------------
// User classes. Use igzstream and ogzstream analogously to ifstream and
// ofstream respectively. They read and write files based on the gz* 
// function interface of the zlib. Files are compatible with gzip compression.
// ----------------------------------------------------------------------------

class igzstream: public gzstreambuf, public istream{
public:
	igzstream() : istream(this) {}
	igzstream(const char* name, int open_mode = ios::in) :
		gzstreambuf(name, open_mode), istream(this) {
	}
	void open(const char* name, int open_mode = ios::in) {
		open(name, open_mode);
	}
	istream& seekg (streamoff off, ios::seekdir way){
		gzstreambuf::seek(off, way);
		return *this;
	}
	streampos tellg (){
		return gzstreambuf::tell();
	}
};

class ogzstream: public gzstreambuf, public ostream {
public:
	ogzstream() : ostream(this) {}
	ogzstream(const char* name, int mode = ios::out) :
		gzstreambuf(name, mode), ostream(this) {
	}
	void open(const char* name, int open_mode = ios::out) {
		open(name, open_mode);
	}
	void close(){
		close();
	}
	ostream& seekp (streamoff off, ios::seekdir way){
		gzstreambuf::seek(off, way);
		return *this;
	}
	streampos tellp (){
		return gzstreambuf::tell();
	}
};

// Generic ifstream and ofstream, use them analogously to ifstream and ofstream respectively.
// Files having names ending with ".gz" will be decompressed and compressed automatically.
// Filenames matching "-"/stdio_filename will be mapped to standard input/output automatically.
class igzfstream: public ifstream {
	gzstreambuf gzBuf;
public:
	igzfstream(){}
	igzfstream(const char *name, ios::openmode open_mode = ios::in){
		open(name, open_mode);
	}
	igzfstream(const string &name, ios::openmode open_mode = ios::in){
		open(name.c_str(), open_mode);
	}
	void open(const string &name, ios::openmode open_mode = ios::in) {
		if (isGzipFilename(name)){
			gzstreambuf* res = gzBuf.open(name.c_str(), open_mode);
			ios::rdbuf(&gzBuf);
			if(!res)
				ios::setstate(ios::failbit|ios::badbit);
		}else if(name==stdio_filename){
			ios::rdbuf(cin.rdbuf());
		}else{
			ifstream::open(name.c_str(), open_mode);
		}
	}
	streambuf* rdbuf(){
		return istream::rdbuf();
	}
	igzfstream& seekg (streamoff off, ios::seekdir way){
		if(gzBuf.is_open())
			gzBuf.seek(off, way);
		else
			ifstream::seekg(off, way);
		return *this;
	}
	streampos tellg (){
		return gzBuf.is_open()? gzBuf.tell() : ifstream::tellg();
	}
};

class ogzfstream: public ofstream {
	gzstreambuf gzBuf;
public:
	ogzfstream(){}
	~ogzfstream(){close();}
	ogzfstream(const char *name, ios::openmode open_mode = ios::out){
		open(name, open_mode);
	}
	ogzfstream(const string &name, ios::openmode open_mode = ios::out){
		open(name.c_str(), open_mode);
	}
	void open(const string &name, ios_base::openmode open_mode = ios::out) {
		if(isGzipFilename(name)){
			gzstreambuf* res = gzBuf.open(name.c_str(), open_mode);
			ios::rdbuf(&gzBuf);
			if(!res)
				ios::setstate(ios::failbit|ios::badbit);
		}else if(name==stdio_filename){
			ios::rdbuf(cout.rdbuf());
		}else{
			ofstream::open(name.c_str(), open_mode);
		}
	}
	streambuf* rdbuf(){
		return ostream::rdbuf();
	}
	ogzfstream& seekp (streamoff off, ios::seekdir way){
		if(gzBuf.is_open())
			gzBuf.seek(off, way);
		else
			ofstream::seekp(off, way);
		return *this;
	}
	streampos tellp (){
		return gzBuf.is_open()? gzBuf.tell() : ofstream::tellp();
	}
};

void CopyFile(const string &fn_in, const string &fn_out);


