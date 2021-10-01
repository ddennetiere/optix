#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "30";
	static const char MONTH[] = "09";
	static const char YEAR[] = "2021";
	static const char UBUNTU_VERSION_STYLE[] =  "21.09";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 1;
	static const long MINOR  = 2;
	static const long BUILD  = 222;
	static const long REVISION  = 225;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 418;
	#define RC_FILEVERSION 1,2,222,225
	#define RC_FILEVERSION_STRING "1, 2, 222, 225\0"
	static const char FULLVERSION_STRING [] = "1.2.222.225";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 22;
	

}
#endif //VERSION_H
