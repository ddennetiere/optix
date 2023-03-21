#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "21";
	static const char MONTH[] = "03";
	static const char YEAR[] = "2023";
	static const char UBUNTU_VERSION_STYLE[] =  "23.03";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 3;
	static const long BUILD  = 383;
	static const long REVISION  = 383;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 423;
	#define RC_FILEVERSION 2,3,383,383
	#define RC_FILEVERSION_STRING "2, 3, 383, 383\0"
	static const char FULLVERSION_STRING [] = "2.3.383.383";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 246;
	

}
#endif //VERSION_H
