#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "13";
	static const char MONTH[] = "04";
	static const char YEAR[] = "2023";
	static const char UBUNTU_VERSION_STYLE[] =  "23.04";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 3;
	static const long BUILD  = 492;
	static const long REVISION  = 492;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 569;
	#define RC_FILEVERSION 2,3,492,492
	#define RC_FILEVERSION_STRING "2, 3, 492, 492\0"
	static const char FULLVERSION_STRING [] = "2.3.492.492";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 355;
	

}
#endif //VERSION_H
