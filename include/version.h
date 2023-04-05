#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "05";
	static const char MONTH[] = "04";
	static const char YEAR[] = "2023";
	static const char UBUNTU_VERSION_STYLE[] =  "23.04";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 3;
	static const long BUILD  = 465;
	static const long REVISION  = 465;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 535;
	#define RC_FILEVERSION 2,3,465,465
	#define RC_FILEVERSION_STRING "2, 3, 465, 465\0"
	static const char FULLVERSION_STRING [] = "2.3.465.465";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 328;
	

}
#endif //VERSION_H
