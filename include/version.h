#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "28";
	static const char MONTH[] = "09";
	static const char YEAR[] = "2021";
	static const char UBUNTU_VERSION_STYLE[] =  "21.09";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 1;
	static const long MINOR  = 2;
	static const long BUILD  = 213;
	static const long REVISION  = 216;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 406;
	#define RC_FILEVERSION 1,2,213,216
	#define RC_FILEVERSION_STRING "1, 2, 213, 216\0"
	static const char FULLVERSION_STRING [] = "1.2.213.216";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 13;
	

}
#endif //VERSION_H
