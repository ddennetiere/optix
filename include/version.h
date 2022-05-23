#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "23";
	static const char MONTH[] = "05";
	static const char YEAR[] = "2022";
	static const char UBUNTU_VERSION_STYLE[] =  "22.05";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 1;
	static const long BUILD  = 611;
	static const long REVISION  = 611;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 2017;
	#define RC_FILEVERSION 2,1,611,611
	#define RC_FILEVERSION_STRING "2, 1, 611, 611\0"
	static const char FULLVERSION_STRING [] = "2.1.611.611";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 287;
	

}
#endif //VERSION_H
