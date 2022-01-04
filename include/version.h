#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "04";
	static const char MONTH[] = "01";
	static const char YEAR[] = "2022";
	static const char UBUNTU_VERSION_STYLE[] =  "22.01";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 1;
	static const long MINOR  = 4;
	static const long BUILD  = 430;
	static const long REVISION  = 433;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 729;
	#define RC_FILEVERSION 1,4,430,433
	#define RC_FILEVERSION_STRING "1, 4, 430, 433\0"
	static const char FULLVERSION_STRING [] = "1.4.430.433";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 30;
	

}
#endif //VERSION_H
