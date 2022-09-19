#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "19";
	static const char MONTH[] = "09";
	static const char YEAR[] = "2022";
	static const char UBUNTU_VERSION_STYLE[] =  "22.09";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 1;
	static const long BUILD  = 681;
	static const long REVISION  = 681;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 2091;
	#define RC_FILEVERSION 2,1,681,681
	#define RC_FILEVERSION_STRING "2, 1, 681, 681\0"
	static const char FULLVERSION_STRING [] = "2.1.681.681";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 357;
	

}
#endif //VERSION_H
