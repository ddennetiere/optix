#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "09";
	static const char MONTH[] = "11";
	static const char YEAR[] = "2021";
	static const char UBUNTU_VERSION_STYLE[] =  "21.11";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 1;
	static const long MINOR  = 4;
	static const long BUILD  = 414;
	static const long REVISION  = 417;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 712;
	#define RC_FILEVERSION 1,4,414,417
	#define RC_FILEVERSION_STRING "1, 4, 414, 417\0"
	static const char FULLVERSION_STRING [] = "1.4.414.417";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 14;
	

}
#endif //VERSION_H
