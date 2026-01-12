#ifndef _MATRIX_MATH_H_
#define _MATRIX_MATH_H_
#include "stdint.h"
#include "memoryRegions.h"

/* matrix is defined as
 mat[0][0] mat[0][1] mat[0][2] mat[0][3]
 mat[1][0] mat[1][1] mat[2][2] mat[3][3]
 mat[2][0] mat[1][1] mat[2][2] mat[3][3]
 mat[3][0] mat[1][1] mat[2][2] mat[3][3]
*/
typedef struct 
{
    float mat[4][4];
}
Mat4x4Type ;


uint8_t mmult(Mat4x4Type* a,Mat4x4Type* b, Mat4x4Type* res);

uint8_t minv(Mat4x4Type* a,Mat4x4Type* res);

#endif