#ifndef SRC_DEBUG_H_
#define SRC_DEBUG_H_

//#define DEBUG 1

#include <stdio.h>
#include <string.h>
#include <time.h>

#define CALCULATE_ELAPSED_TIME \
	struct timespec t_; \
	clock_gettime(CLOCK_MONOTONIC, &t_); \
	double time_elapsed_ = t_.tv_sec + t_.tv_nsec / 1000000000.0;


#ifdef DEBUG
#define debug(msg) do { CALCULATE_ELAPSED_TIME; fprintf(stderr, "[%30s][%20s] %18.6f: %s\n", __FILE__, __func__, time_elapsed_, msg); } while(0)
#define fdebug(format, ...) do {\
	CALCULATE_ELAPSED_TIME \
	char format_[80]; \
	strcpy(format_, "[%30s][%20s] %18.6f: "); \
	strcat(format_, format); \
	strcat(format_, "\n"); \
	fprintf(stderr, format_,  __FILE__, __func__, time_elapsed_, __VA_ARGS__); \
} while(0)
#else
#define debug(msg) do { CALCULATE_ELAPSED_TIME; fprintf(stderr, "%18.6f: %s\n", time_elapsed_, msg); } while(0)
#define fdebug(format, ...) do {\
	CALCULATE_ELAPSED_TIME \
	char format_[80]; \
	strcpy(format_, "%18.6f: "); \
	strcat(format_, format); \
	strcat(format_, "\n"); \
	fprintf(stderr, format_, time_elapsed_, __VA_ARGS__); \
} while(0)
#endif
#endif /* SRC_DEBUG_H_ */
