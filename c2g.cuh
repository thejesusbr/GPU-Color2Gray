/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

/* Template project which demonstrates the basics on how to setup a project 
 * example application.
 * Device code.
 */

#ifndef _C2G_KERNEL_H_
#define _C2G_KERNEL_H_

#include <stdio.h>

#include <cuda.h>
#include <cuda_runtime.h>


// includes, project
#include <cutil_inline.h>

#define BLOCK_TILE 16
#define BLOCK_1D 256 //BLOCK_TILE * BLOCK_TILE
#define MAX_SH_X 32
#define MAX_SH_Y 32
#define SH_1D BLOCK_1D * 4 // 16*16*4

#define CRUNCH(alfa, x) (alfa) * tanhf(x/alfa)

#define DOT(a, b, ctetha, stetha) (a) * (ctetha) + (b) * (stetha)

#define L2NORM(x, y) sqrtf(((x)*(x))+((y)*(y)))

void cuColor2Gray(amy_lab *dataIn, float *dataOut, int n, float alpha, float ctheta, float stheta);

#endif // #ifndef _C2G_KERNEL_H_
