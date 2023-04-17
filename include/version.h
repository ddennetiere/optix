#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "17";
	static const char MONTH[] = "04";
	static const char YEAR[] = "2023";
	static const char UBUNTU_VERSION_STYLE[] =  "23.04";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 3;
	static const long BUILD  = 459;
	static const long REVISION  = 459;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 526;
	#define RC_FILEVERSION 2,3,459,459
	#define RC_FILEVERSION_STRING "2, 3, 459, 459\0"
	static const char FULLVERSION_STRING [] = "2.3.459.459";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 322;
	

}
#endif //VERSION_H
