#ifndef PX_MEMORY_H
#define PX_MEMORY_H

/*
	px_memory.h

	some helpful macros for debbuging malloc and free calls

*/

#define px_malloc(a) \
	malloc(a);   \
	printf("malloc at %s line: %d in %s \n", __func__, __LINE__, __FILE__); \

#define px_free(a) \
	free(a);   \
	printf("free at %s line: %d in %s \n", __func__, __LINE__, __FILE__); \


#endif
