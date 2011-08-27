#ifndef IMAGES_H
#define IMAGES_H

#include <stdexcept>
#include <vector>
#include <algorithm>
#include "amy_colors.h"

using std::vector;

extern float alpha;
extern float theta;

extern bool quantize;
extern int q_colors;


inline float crunch(float chrom_dist) {
	return alpha==0 ? 0 : alpha*tanh(chrom_dist/alpha);
}

inline float sq(float s) { return s*s; }

struct ColorImage {

	amy_lab * data;
	amy_yCrCb *dataYCrCb;
	
	typedef std::pair<amy_lab,int> amy_lab_int;
	vector<amy_lab_int> qdata;
	
	int colors;
	
	int w, h, N;

	ColorImage() : data(NULL) {}
	void clean() { delete [] data; }

	float calc_delta(int i, int j) const;
	float calc_qdelta(int i, int p) const;

	void load_quant_data(const char *fname);
	
	float * calc_d();

	float * r_calc_d(int r);
	
	void load(const char * fname);

	void save(const char * fname, bool type = true) const;

};

struct GrayImage {

	float * data;
	const int w, h, N;
	int i,j,k;

	//this will shift our data to best match the 
	//luminance channel of s.
	void post_solve(const ColorImage &s);

	GrayImage( ColorImage &s) : data(new float[s.N]), w(s.w), h(s.h), N(s.N) {
		for(i=0;i<N;i++) 
			data[i]=(s.data)[i].l;
	}
	~GrayImage() { delete [] data; }

	void complete_solve(const float *d);
	void r_solve(const float *d, int r);
	
	void save(const char * fname) const;
	void saveColor(const char * fname, const ColorImage &source) const;
};	

#endif
