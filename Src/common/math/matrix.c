#include "matrixMath.h"
#include "memoryRegions.h"
__ITCM_CODE
uint8_t mmult(Mat4x4Type*  a, Mat4x4Type* b,Mat4x4Type* res)
{
    res->mat[0][0] =  a->mat[0][0]*b->mat[0][0] + a->mat[0][1]*b->mat[1][0] + a->mat[0][2]*b->mat[2][0] + a->mat[0][3]*b->mat[3][0];
    res->mat[0][1] =  a->mat[0][0]*b->mat[0][1] + a->mat[0][1]*b->mat[1][1] + a->mat[0][2]*b->mat[2][1] + a->mat[0][3]*b->mat[3][1];
    res->mat[0][2] =  a->mat[0][0]*b->mat[0][2] + a->mat[0][1]*b->mat[1][2] + a->mat[0][2]*b->mat[2][2] + a->mat[0][3]*b->mat[3][2];
    res->mat[0][3] =  a->mat[0][0]*b->mat[0][3] + a->mat[0][1]*b->mat[1][3] + a->mat[0][2]*b->mat[2][3] + a->mat[0][3]*b->mat[3][3];

    res->mat[1][0] =  a->mat[1][0]*b->mat[0][0] + a->mat[1][1]*b->mat[1][0] + a->mat[1][2]*b->mat[2][0] + a->mat[1][3]*b->mat[3][0];
    res->mat[1][1] =  a->mat[1][0]*b->mat[0][1] + a->mat[1][1]*b->mat[1][1] + a->mat[1][2]*b->mat[2][1] + a->mat[1][3]*b->mat[3][1];
    res->mat[1][2] =  a->mat[1][0]*b->mat[0][2] + a->mat[1][1]*b->mat[1][2] + a->mat[1][2]*b->mat[2][2] + a->mat[1][3]*b->mat[3][2];
    res->mat[1][3] =  a->mat[1][0]*b->mat[0][3] + a->mat[1][1]*b->mat[1][3] + a->mat[1][2]*b->mat[2][3] + a->mat[1][3]*b->mat[3][3];

    res->mat[2][0] =  a->mat[2][0]*b->mat[0][0] + a->mat[2][1]*b->mat[1][0] + a->mat[2][2]*b->mat[2][0] + a->mat[2][3]*b->mat[3][0];
    res->mat[2][1] =  a->mat[2][0]*b->mat[0][1] + a->mat[2][1]*b->mat[1][1] + a->mat[2][2]*b->mat[2][1] + a->mat[2][3]*b->mat[3][1];
    res->mat[2][2] =  a->mat[2][0]*b->mat[0][2] + a->mat[2][1]*b->mat[1][2] + a->mat[2][2]*b->mat[2][2] + a->mat[2][3]*b->mat[3][2];
    res->mat[2][3] =  a->mat[2][0]*b->mat[0][3] + a->mat[2][1]*b->mat[1][3] + a->mat[2][2]*b->mat[2][3] + a->mat[2][3]*b->mat[3][3];

    res->mat[3][0] =  a->mat[3][0]*b->mat[0][0] + a->mat[3][1]*b->mat[1][0] + a->mat[3][2]*b->mat[2][0] + a->mat[3][3]*b->mat[3][0];
    res->mat[3][1] =  a->mat[3][0]*b->mat[0][1] + a->mat[3][1]*b->mat[1][1] + a->mat[3][2]*b->mat[2][1] + a->mat[3][3]*b->mat[3][1];
    res->mat[3][2] =  a->mat[3][0]*b->mat[0][2] + a->mat[3][1]*b->mat[1][2] + a->mat[3][2]*b->mat[2][2] + a->mat[3][3]*b->mat[3][2];
    res->mat[3][3] =  a->mat[3][0]*b->mat[0][3] + a->mat[3][1]*b->mat[1][3] + a->mat[3][2]*b->mat[2][3] + a->mat[3][3]*b->mat[3][3];
    return 0;
}

__ITCM_CODE
uint8_t minv(Mat4x4Type* a,Mat4x4Type* res)
{
    float f_11_22_33 = a->mat[1][1]*a->mat[2][2]*a->mat[3][3];
    float f_12_23_31 = a->mat[1][2]*a->mat[2][3]*a->mat[3][1];
    float f_13_21_32 = a->mat[1][3]*a->mat[2][1]*a->mat[3][2];
    float f_13_22_31 = a->mat[1][3]*a->mat[2][2]*a->mat[3][1];
    float f_12_21_33 = a->mat[1][2]*a->mat[2][1]*a->mat[3][3];
    float f_11_23_32 = a->mat[1][1]*a->mat[2][3]*a->mat[3][2];

    float f_01_22_33 = a->mat[0][1]*a->mat[2][2]*a->mat[3][3];
    float f_02_23_31 = a->mat[0][2]*a->mat[2][3]*a->mat[3][1];
    float f_03_21_32 = a->mat[0][3]*a->mat[2][1]*a->mat[3][2];
    float f_03_22_31 = a->mat[0][3]*a->mat[2][2]*a->mat[3][1];
    float f_02_21_33 = a->mat[0][2]*a->mat[2][1]*a->mat[3][3];
    float f_01_23_32 = a->mat[0][1]*a->mat[2][3]*a->mat[3][2];

    float f_01_12_33 = a->mat[0][1]*a->mat[1][2]*a->mat[3][3];
    float f_02_13_31 = a->mat[0][2]*a->mat[1][3]*a->mat[3][1];
    float f_03_12_31 = a->mat[0][3]*a->mat[1][2]*a->mat[3][1];
    float f_03_11_32 = a->mat[0][3]*a->mat[1][1]*a->mat[3][2];
    float f_02_11_33 = a->mat[0][2]*a->mat[1][1]*a->mat[3][3];
    float f_01_13_32 = a->mat[0][1]*a->mat[1][3]*a->mat[3][2];

    float f_01_12_23 = a->mat[0][1]*a->mat[1][2]*a->mat[2][3];
    float f_02_13_21 = a->mat[0][2]*a->mat[1][3]*a->mat[2][1];
    float f_03_11_22 = a->mat[0][3]*a->mat[1][1]*a->mat[2][2];
    float f_03_12_21 = a->mat[0][3]*a->mat[1][2]*a->mat[2][1];
    float f_02_11_23 = a->mat[0][2]*a->mat[1][1]*a->mat[2][3];
    float f_01_13_22 = a->mat[0][1]*a->mat[1][3]*a->mat[2][2];
    float fact_0_0 = f_11_22_33 + 
                     f_12_23_31 +
                     f_13_21_32 -
                     f_13_22_31 - 
                     f_12_21_33 -
                     f_11_23_32;
    
    float fact_0_1 = -f_01_22_33 - 
                      f_02_23_31 -
                      f_03_21_32 +
                      f_03_22_31 + 
                      f_02_21_33 +
                      f_01_23_32;

    float fact_0_2 = f_01_12_33 + 
                     f_02_13_31 +
                     f_03_11_32 -
                     f_03_12_31 - 
                     f_02_11_33 -
                     f_01_13_32;
                     
    float fact_0_3 = -f_01_12_23 - 
                      f_02_13_21 -
                      f_03_11_22 +
                      f_03_12_21 + 
                      f_02_11_23 +
                      f_01_13_22;

    float determinant = a->mat[0][0]*fact_0_0 +
                        a->mat[1][0]*fact_0_1 + 
                        a->mat[2][0]*fact_0_2 + 
                        a->mat[3][0]*fact_0_3; 
    if (determinant > 0.00001f || determinant < -0.00001f) 
    {
        res->mat[0][0] = fact_0_0;
        res->mat[0][1] = fact_0_1; 
        res->mat[0][2] = fact_0_2;    
        res->mat[0][3] = fact_0_3;   
        float f_10_22 = a->mat[1][0]*a->mat[2][2];
        float f_23_31 = a->mat[2][3]*a->mat[3][0];
        float f_13_20 = a->mat[1][3]*a->mat[2][0];
        float f_22_30 = a->mat[2][2]*a->mat[3][0];
        float f_12_20 = a->mat[1][2]*a->mat[2][0];
        float f_10_23 = a->mat[1][0]*a->mat[2][3];
        float f_00_22 = a->mat[0][0]*a->mat[2][2];
        float f_20_32 = a->mat[2][0]*a->mat[3][2];
        float f_20_33 = a->mat[2][0]*a->mat[3][3];
        float f_00_23 = a->mat[0][0]*a->mat[2][3];
        float f_00_12 = a->mat[0][0]*a->mat[1][2];
        float f_13_30 = a->mat[1][3]*a->mat[3][0];
        float f_03_10 = a->mat[0][3]*a->mat[1][0];
        float f_12_30 = a->mat[1][2]*a->mat[3][0];
        float f_02_10 = a->mat[0][2]*a->mat[1][0];
        float f_00_13 = a->mat[0][0]*a->mat[1][3];
        float f_10_21 = a->mat[1][0]*a->mat[2][1];
        float f_21_30 = a->mat[2][1]*a->mat[3][0];
        float f_00_21 = a->mat[0][0]*a->mat[2][1];
        float f_20_31 = a->mat[2][0]*a->mat[3][1];
        float f_00_11 = a->mat[0][0]*a->mat[1][1];
        float f_11_30 = a->mat[1][1]*a->mat[3][0];
        float f_01_10 = a->mat[0][1]*a->mat[1][0];
        float f_11_20 = a->mat[1][1]*a->mat[2][0];

        res->mat[1][0] = -f_10_22*a->mat[3][3] - 
                        a->mat[1][2]*f_23_31 -
                        f_13_20*a->mat[3][2] +
                        a->mat[1][3]*f_22_30 + 
                        f_12_20*a->mat[3][3] +
                        f_10_23*a->mat[3][2];

        res->mat[1][1] = f_00_22*a->mat[3][3] + 
                        a->mat[0][2]*f_23_31 +
                        a->mat[0][3]*f_20_32 -
                        a->mat[0][3]*f_22_30 - 
                        a->mat[0][2]*f_20_33 -
                        f_00_23*a->mat[3][2];

        res->mat[1][2] = -f_00_12*a->mat[3][3] - 
                        a->mat[0][2]*f_13_30 -
                        f_03_10*a->mat[3][2] +
                        a->mat[0][3]*f_12_30 + 
                        f_02_10*a->mat[3][3] +
                        f_00_13*a->mat[3][2];    
                        
        res->mat[1][3] = f_00_12*a->mat[2][3] + 
                        a->mat[0][2]*f_13_20 +
                        a->mat[0][3]*f_10_22 -
                        a->mat[0][3]*f_12_20 - 
                        a->mat[0][2]*f_10_23 -
                        f_00_13*a->mat[2][2]; 

        res->mat[2][0] = f_10_21*a->mat[3][3] + 
                        a->mat[1][1]*f_23_31 +
                        f_13_20*a->mat[3][1] -
                        a->mat[1][3]*f_21_30 - 
                        a->mat[1][1]*f_20_33 -
                        f_10_23*a->mat[3][1];

        res->mat[2][1] = -f_00_21*a->mat[3][3] - 
                        a->mat[0][1]*f_23_31 -
                        a->mat[0][3]*f_20_31 +
                        a->mat[0][3]*f_21_30 + 
                        a->mat[0][1]*f_20_33 +
                        f_00_23*a->mat[3][1];

        res->mat[2][2] = f_00_11*a->mat[3][3] + 
                        a->mat[0][1]*f_13_30 +
                        f_03_10*a->mat[3][1] -
                        a->mat[0][3]*f_11_30 - 
                        f_01_10*a->mat[3][3] -
                        f_00_13*a->mat[3][1];    
                        
        res->mat[2][3] = -f_00_11*a->mat[2][3] - 
                        a->mat[0][1]*f_13_20 -
                        f_03_10*a->mat[2][1] +
                        a->mat[0][3]*f_11_20 + 
                        a->mat[0][1]*f_10_23 +
                        f_00_13*a->mat[2][1]; 

        res->mat[3][0] = -f_10_21*a->mat[3][2] - 
                        a->mat[1][1]*f_22_30 -
                        f_12_20*a->mat[3][1] +
                        a->mat[1][2]*f_21_30 + 
                        a->mat[1][1]*f_20_32 +
                        f_10_22*a->mat[3][1];

        res->mat[3][1] = f_00_21*a->mat[3][2] + 
                        a->mat[0][1]*f_22_30 +
                        a->mat[0][2]*f_20_31 -
                        a->mat[0][2]*f_21_30 - 
                        a->mat[0][1]*f_20_32 -
                        f_00_22*a->mat[3][1];

        res->mat[3][2] = -f_00_11*a->mat[3][2] - 
                        a->mat[0][1]*f_12_30 -
                        f_02_10*a->mat[3][1] +
                        a->mat[0][2]*f_11_30 + 
                        f_01_10*a->mat[3][2] +
                        f_00_12*a->mat[3][1];    
                        
        res->mat[3][3] = f_00_11*a->mat[2][2] + 
                        a->mat[0][1]*f_12_20 +
                        f_02_10*a->mat[2][1] -
                        a->mat[0][2]*f_11_20 - 
                        a->mat[0][1]*f_10_22 -
                        f_00_12*a->mat[2][1]; 
        res->mat[0][0] /= determinant;
        res->mat[0][1] /= determinant;
        res->mat[0][2] /= determinant;
        res->mat[0][3] /= determinant;

        res->mat[1][0] /= determinant;
        res->mat[1][1] /= determinant;
        res->mat[1][2] /= determinant;
        res->mat[1][3] /= determinant;

        res->mat[2][0] /= determinant;
        res->mat[2][1] /= determinant;
        res->mat[2][2] /= determinant;
        res->mat[2][3] /= determinant;

        res->mat[3][0] /= determinant;
        res->mat[3][1] /= determinant;
        res->mat[3][2] /= determinant;
        res->mat[3][3] /= determinant;
        return 0;
    }




    return 1;
}