#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "28";
	static const char MONTH[] = "12";
	static const char YEAR[] = "2021";
	static const char UBUNTU_VERSION_STYLE[] =  "21.12";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 1;
	static const long MINOR  = 4;
	static const long BUILD  = 419;
	static const long REVISION  = 422;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 717;
	#define RC_FILEVERSION 1,4,419,422
	#define RC_FILEVERSION_STRING "1, 4, 419, 422\0"
	static const char FULLVERSION_STRING [] = "1.4.419.422";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 19;
	

}
#endif //VERSION_H
