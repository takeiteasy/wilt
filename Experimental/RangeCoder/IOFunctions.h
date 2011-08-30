#ifndef __IO_FUNCTIONS_H__
#define __IO_FUNCTIONS_H__

typedef int RangeCoderReadFunction(void *readcontext);
typedef int RangeCoderWriteFunction(int b,void *writecontext);
