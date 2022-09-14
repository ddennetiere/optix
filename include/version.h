#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "14";
	static const char MONTH[] = "09";
	static const char YEAR[] = "2022";
	static const char UBUNTU_VERSION_STYLE[] =  "22.09";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 1;
	static const long BUILD  = 676;
	static const long REVISION  = 676;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 2088;
	#define RC_FILEVERSION 2,1,676,676
	#define RC_FILEVERSION_STRING "2, 1, 676, 676\0"
	static const char FULLVERSION_STRING [] = "2.1.676.676";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 352;
	

}
#endif //VERSION_H
