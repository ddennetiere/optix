#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "28";
	static const char MONTH[] = "06";
	static const char YEAR[] = "2023";
	static const char UBUNTU_VERSION_STYLE[] =  "23.06";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 3;
	static const long BUILD  = 758;
	static const long REVISION  = 758;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 977;
	#define RC_FILEVERSION 2,3,758,758
	#define RC_FILEVERSION_STRING "2, 3, 758, 758\0"
	static const char FULLVERSION_STRING [] = "2.3.758.758";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 621;
	

}
#endif //VERSION_H
