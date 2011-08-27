/*
 * c2g.cu
 *
 *  Created on: 18/07/2011
 *      Author: wendell & agnus
 */

// includes, system
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
//#include <cmath>
#include <iostream>
#include "amy_colors.h"

// includes, kernels
#include <c2g.cuh>

using namespace std;

__host__ __device__ int iDivUp(int a, int b)
{
	return a / b + (a % b != 0);
}

__constant__ float cAlpha, cSTheta, cCTheta;
__constant__ int cN;

__global__ void cuDColor2Gray_kernel(amy_lab *dataIn, float *dataOut)
{

	const int id = (blockDim.x * blockIdx.x) + threadIdx.x;

	if (id >= cN)
		return;

	float sum = 0;
	int nSigma = 0;

#pragma unroll
	for (int j = 0; j < cN; ++j)
	{

		float Lij = dataIn[id].l - dataIn[j].l;

		//Calculo de Lj + deltaij
		if (fabs(Lij) <= cAlpha)
		{
			float dij;

			float Cij[] =
			{ (dataIn[id].a - dataIn[j].a), (dataIn[id].b - dataIn[j].b) };

			float crunch = CRUNCH(cAlpha, L2NORM(Cij[0], Cij[1]));

			if (fabs(Lij) > crunch)
			{
				dij = dataIn[id].l;
			}
			else
			{
				float sign = DOT(Cij[0], Cij[1], cCTheta, cSTheta);
				dij = sign > 0 ? crunch : -crunch;
				dij += dataIn[j].l;
			}
			sum += dij;
			++nSigma;
		}
	}
	dataOut[id] = ((((cN) - nSigma) * dataIn[id].l + sum) / (cN));
	//	dataOut[id] = sum;
}

__global__ void cuColor2Gray_kernel(amy_lab *dataIn, float *dataOut)
{
	const int id = (blockDim.x * blockIdx.x) + threadIdx.x;

	__shared__
	amy_lab shImg[SH_1D];

	float sum = 0;
	int nSigma = 0;

	register float Li = dataIn[id].l;
	register float ai = dataIn[id].a;
	register float bi = dataIn[id].b;
	register unsigned int pixelCount = 0;

	for (int i = 0; i < iDivUp(cN, SH_1D); ++i)
	{
		//shImg[threadIdx.x] = threadIdx.x + i * BLOCK_1D < w * h ? dataIn[threadIdx.x + i * BLOCK_1D] : zero;
		//		for (int k = 0; k < 4; ++k)
		//		{
		//			if (threadIdx.x + i * BLOCK_1D < cN)
		//			{
		//				shImg[threadIdx.x + k*BLOCK_1D] = dataIn[threadIdx.x + i * BLOCK_1D];
		//			}
		//		}
		if (threadIdx.x + i * SH_1D < cN)
		{
			shImg[threadIdx.x] = dataIn[threadIdx.x + i * SH_1D];
			shImg[threadIdx.x + BLOCK_1D] = dataIn[threadIdx.x + i * SH_1D + BLOCK_1D];
			shImg[threadIdx.x + BLOCK_1D*2] = dataIn[threadIdx.x + i * SH_1D + BLOCK_1D*2];
			shImg[threadIdx.x + BLOCK_1D*3] = dataIn[threadIdx.x + i * SH_1D + BLOCK_1D*3];

		__syncthreads();
		for (int j = 0; j < SH_1D && pixelCount < cN; ++j)
		{
			float Lij = Li - shImg[j].l;

			//Calculo de Lj + deltaij
			if (abs(Lij) <= cAlpha)
			{
				float dij;

				float Cij[] =
				{ (ai - shImg[j].a), (bi - shImg[j].b) };

				float crunch = CRUNCH(cAlpha, L2NORM(Cij[0], Cij[1]));

				if (abs(Lij) > crunch)
				{
					dij = Li;
				}
				else
				{
					float sign = DOT(Cij[0], Cij[1], cCTheta, cSTheta);
					dij = sign > 0 ? crunch : -crunch;
					dij += shImg[j].l;
				}
				sum += dij;
				++nSigma;
			}
			++pixelCount;
		}
		__syncthreads();
		}
	}
	if(id >= cN) return;
	dataOut[id] = ((((cN) - nSigma) * Li + sum) / (cN));
}

void cuColor2Gray(amy_lab *dataIn, float *dataOut, int n, float alpha,
		float ctheta, float stheta)
{

	int grid(iDivUp(n, BLOCK_1D));
	int block(BLOCK_1D);

	//	cout << "G: " << grid << " B: " << block << endl;

	//Ponteiros para o device
	amy_lab *devDataIn;
	float *devDataOut;

	size_t sizeAmy = n * sizeof(amy_lab);
	size_t sizeF = n * sizeof(float);

	cudaMalloc((void **) &devDataIn, sizeAmy);
	cudaMalloc((void **) &devDataOut, sizeF);

	cudaMemcpy(devDataIn, dataIn, sizeAmy, cudaMemcpyHostToDevice);
	cudaMemcpy(devDataOut, dataOut, sizeF, cudaMemcpyHostToDevice);

	cudaMemcpyToSymbol(cN, &n, sizeof(int));
	cudaMemcpyToSymbol(cAlpha, &alpha, sizeof(float));
	cudaMemcpyToSymbol(cSTheta, &stheta, sizeof(float));
	cudaMemcpyToSymbol(cCTheta, &ctheta, sizeof(float));

	//	cudaMemset(devDataOut, 1, sizeF);

	//	cout << "dataIn:" << endl;
	//	for (int i = 0; i < width * height; ++i)
	//	{
	//		cout << dataIn[i].l << " ";
	//		if ((i + 1) % width == 0)
	//			cout << endl;
	//	}

	cuColor2Gray_kernel<<<grid, block>>>(devDataIn, devDataOut);

	cudaMemcpy(dataOut, devDataOut, sizeF, cudaMemcpyDeviceToHost);

	//	cout << "dataOut:" << endl;
	//	for (int i = 0; i < n; ++i)
	//	{
	//		cout << dataOut[i] << " ";
	//		if ((i + 1) % 100 == 0)
	//			cout << endl;
	//	}

	//	cout << "Diff:" << endl;
	//	for (int i = 0; i < width*height; ++i) {
	//			cout << dataOut[i] - dataIn[i].l << " ";
	//			if((i+1)% width == 0) cout << endl;
	//		}

	cudaFree(devDataIn);
	cudaFree(devDataOut);

}

