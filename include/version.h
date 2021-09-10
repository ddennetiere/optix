#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "10";
	static const char MONTH[] = "09";
	static const char YEAR[] = "2021";
	static const char UBUNTU_VERSION_STYLE[] =  "21.09";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 1;
	static const long MINOR  = 1;
	static const long BUILD  = 135;
	static const long REVISION  = 138;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 270;
	#define RC_FILEVERSION 1,1,135,138
	#define RC_FILEVERSION_STRING "1, 1, 135, 138\0"
	static const char FULLVERSION_STRING [] = "1.1.135.138";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 35;
	

}
#endif //VERSION_H
