#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "08";
	static const char MONTH[] = "02";
	static const char YEAR[] = "2022";
	static const char UBUNTU_VERSION_STYLE[] =  "22.02";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 1;
	static const long MINOR  = 6;
	static const long BUILD  = 672;
	static const long REVISION  = 675;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 1065;
	#define RC_FILEVERSION 1,6,672,675
	#define RC_FILEVERSION_STRING "1, 6, 672, 675\0"
	static const char FULLVERSION_STRING [] = "1.6.672.675";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 72;
	

}
#endif //VERSION_H
