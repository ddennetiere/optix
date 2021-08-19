#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "19";
	static const char MONTH[] = "08";
	static const char YEAR[] = "2021";
	static const char UBUNTU_VERSION_STYLE[] =  "21.08";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 1;
	static const long MINOR  = 0;
	static const long BUILD  = 76;
	static const long REVISION  = 79;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 184;
	#define RC_FILEVERSION 1,0,76,79
	#define RC_FILEVERSION_STRING "1, 0, 76, 79\0"
	static const char FULLVERSION_STRING [] = "1.0.76.79";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 76;
	

}
#endif //VERSION_H
