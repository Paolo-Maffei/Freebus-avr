
#ifndef _CPPSTUFF_H_
#define _CPPSTUFF_H_

#include "stdlib.h"

//Some missing C++ operations
inline void* operator new(unsigned int size) { return malloc(size); }
inline void* operator new(unsigned int size, void* ptr){ return realloc(ptr,size);}
inline void operator delete(void* ptr) { free(ptr); }
inline void* operator new[](unsigned int size ) { return malloc(size);}
inline void* operator new[](unsigned int size, void* ptr){ return realloc(ptr,size);}
inline void operator delete[](void* ptr) { free(ptr); }

//Some helper macros
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)



/* for "warning: only initialized variables can be placed into program memory area"
   see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
   it's an compiler bug in c++. The produced code is ok.
*/


#endif // _CPPSTUFF_H_
 
