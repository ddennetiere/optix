#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "10";
	static const char MONTH[] = "02";
	static const char YEAR[] = "2022";
	static const char UBUNTU_VERSION_STYLE[] =  "22.02";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 1;
	static const long MINOR  = 6;
	static const long BUILD  = 712;
	static const long REVISION  = 715;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 1108;
	#define RC_FILEVERSION 1,6,712,715
	#define RC_FILEVERSION_STRING "1, 6, 712, 715\0"
	static const char FULLVERSION_STRING [] = "1.6.712.715";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 112;
	

}
#endif //VERSION_H
