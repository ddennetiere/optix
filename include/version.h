#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "31";
	static const char MONTH[] = "08";
	static const char YEAR[] = "2021";
	static const char UBUNTU_VERSION_STYLE[] =  "21.08";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 1;
	static const long MINOR  = 0;
	static const long BUILD  = 89;
	static const long REVISION  = 92;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 206;
	#define RC_FILEVERSION 1,0,89,92
	#define RC_FILEVERSION_STRING "1, 0, 89, 92\0"
	static const char FULLVERSION_STRING [] = "1.0.89.92";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 89;
	

}
#endif //VERSION_H
