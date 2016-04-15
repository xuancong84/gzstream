#include "gzstream.h"
#include <iostream>
#include <fstream>
#include <string.h>  // for memcpy

using namespace std;

// ----------------------------------------------------------------------------
// Internal classes to implement gzstream. See header file for user classes.
// ----------------------------------------------------------------------------

// --------------------------------------
// class gzstreambuf:
// --------------------------------------
bool isGzipFilename(string name){
	if (name.size() > 3){
		transform(name.begin(), name.end(), name.begin(), ::tolower);
		if (name.substr(name.size() - 3, 3) == ".gz")
			return true;
	}
	return false;
}

gzstreambuf* gzstreambuf::open(const char* name, int open_mode) {
	if (is_open())
		return (gzstreambuf*) 0;
	mode = open_mode;
	// no append nor read/write mode
	if ((mode & std::ios::ate) || (mode & std::ios::app) || ((mode & std::ios::in) && (mode & std::ios::out)))
		return (gzstreambuf*) 0;
	char fmode[10];
	char* fmodeptr = fmode;
	if (mode & std::ios::in)
		*fmodeptr++ = 'r';
	else if (mode & std::ios::out)
		*fmodeptr++ = 'w';
	*fmodeptr++ = 'b';
	*fmodeptr = '\0';
	file = gzopen(name, fmode);
	if (file == 0)
		return (gzstreambuf*) 0;
	opened = 1;
	return this;
}

gzstreambuf * gzstreambuf::close() {
	if (is_open()) {
		sync();
		opened = 0;
		if (gzclose(file) == Z_OK)
			return this;
	}
	return (gzstreambuf*) 0;
}

int gzstreambuf::underflow() { // used for input buffer only
	if (gptr() && (gptr() < egptr()))
		return *reinterpret_cast<unsigned char *>(gptr());

	if (!(mode & std::ios::in) || !opened)
		return EOF;
	// Josuttis' implementation of inbuf
	int n_putback = gptr() - eback();
	if (n_putback > 4)
		n_putback = 4;
	memcpy(buffer + (4 - n_putback), gptr() - n_putback, n_putback);

	int num = gzread(file, buffer + 4, bufferSize - 4);
	if (num <= 0) // ERROR or EOF
		return EOF;

	// reset buffer pointers
	setg(buffer + (4 - n_putback),   // beginning of putback area
	buffer + 4,                 // read position
	buffer + 4 + num);          // end of buffer

	// return next character
	return *reinterpret_cast<unsigned char *>(gptr());
}

int gzstreambuf::flush_buffer() {
	// Separate the writing of the buffer from overflow() and
	// sync() operation.
	int w = pptr() - pbase();
	if (gzwrite(file, pbase(), w) != w)
		return EOF;
	pbump(-w);
	return w;
}

int gzstreambuf::overflow(int c) { // used for output buffer only
	if (!(mode & std::ios::out) || !opened)
		return EOF;
	if (c != EOF) {
		*pptr() = c;
		pbump(1);
	}
	if (flush_buffer() == EOF)
		return EOF;
	return c;
}

int gzstreambuf::sync() {
	// Changed to use flush_buffer() instead of overflow( EOF)
	// which caused improper behavior with std::endl and flush(),
	// bug reported by Vincent Ricard.
	if (pptr() && pptr() > pbase()) {
		if (flush_buffer() == EOF)
			return -1;
	}
	return 0;
}

void CopyFile(const string &fn_in, const string &fn_out) {
	ifstream fin(fn_in.c_str(), ios::binary);
	ofstream fout(fn_out.c_str(), ios::binary);

	fout << fin.rdbuf();

	fin.close();
	fout.close();
}


#ifdef DEBUG_GZIP
#include <sstream>
ostream &write_big(ostream &os){
	for(int x=0; x<100; x++){
		for(int y=0; y<1000; y++)
			os << x+y << ' ';
		os << endl << '\0';
	}
	os << "hello world!" << endl;
	return os;
}

void checks(istream &is){
	cerr << "checks: ";
	ostringstream oss1, oss2;
	streampos length = write_big(oss1).tellp();
	oss2 << is.rdbuf();
	if(oss2.tellp()!=length){
		cerr << "Error: size is different: " << length << ' ' << oss2.tellp() << endl;
		return;
	}
	if(memcmp(oss1.str().c_str(), oss2.str().c_str(), length))
		cerr << "Error: data is different" << endl;
	else
		cerr << "Success! length=" << length << endl;
}

void checkfs(igzfstream &fs){
	cerr << "checkfs: ";
	ostringstream oss;
	streampos length = write_big(oss).tellp();
	fs.seekg(0, ios::end);
	streampos posi = fs.tellg();
	fs.seekg(0, ios::beg);
	if(posi!=length){
		cerr << "Error: size is different: " << length << ' ' << posi << endl;
		return;
	}
	char *buf = new char [posi];
	fs.read(buf, posi);
	if(memcmp(oss.str().data(), buf, length))
		cerr << "Error: data is different" << endl;
	else
		cerr << "Success! length=" << length << endl;
	delete [] buf;
}

void checkis(igzstream &fs){
	cerr << "checkfs: ";
	ostringstream oss;
	streampos length = write_big(oss).tellp();
	fs.seekg(0, ios::end);
	streampos posi = fs.tellg();
	fs.seekg(0, ios::beg);
	if(posi!=length){
		cerr << "Error: size is different: " << length << ' ' << posi << endl;
		return;
	}
	char *buf = new char [posi];
	fs.read(buf, posi);
	if(memcmp(oss.str().data(), buf, length))
		cerr << "Error: data is different" << endl;
	else
		cerr << "Success! length=" << length << endl;
	delete [] buf;
}

void do1(){
    ogzfstream f("/tmp/a");
    write_big(f);
}

void do2(){
    ogzfstream f("/tmp/a.gz");
    write_big(f);
}

void do3(){
    ogzstream f("/tmp/b.gz");
    write_big(f);
}

string s;
void do1a(){
    igzfstream f("/tmp/a");
    checks(f);
	checkfs(f);
}

void do2a(){
    igzfstream f("/tmp/a.gz");
    checks(f);
	checkfs(f);
}

void do3a(){
    igzfstream f("/tmp/b.gz");
    checks(f);
	checkfs(f);
}

void do1b(){
    igzstream f("/tmp/a.gz");
    checks(f);
	checkis(f);
}

int main(){
    do1();
    do2();
    do3();

    do1a();
    do2a();
    do3a();
	
	do1b();

    return  0;
}

#endif

// ============================================================================
// EOF //
