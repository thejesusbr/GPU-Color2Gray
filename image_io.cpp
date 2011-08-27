#include <stdio.h>
#include <string.h>
#include <map>
#include "images.h"

//return variables for fscanf and fread to keep the compiler happy
static int ret = 0;
static size_t bytesRead = 0;

sven::rgb *readPPM(const char *filename, int *cols, int *rows, int * colors)
{
	using sven::rgb;
	char tag[40];
	rgb *image;
	FILE *fp;
	int read, num[3], curchar;

	if (filename != NULL && strlen(filename))
		fp = fopen(filename, "rb");
	else
		fp = stdin;

	if (fp)
	{
		ret = fscanf(fp, "%s\n", tag);
		bool binary;

		// Read the "magic number" at the beginning of the ppm
		if (strncmp(tag, "P6", 40) == 0)
			binary = true;
		else if (strncmp(tag, "P3", 40) == 0)
			binary = false;
		else
			throw std::runtime_error("not a ppm!");

		// Read the rows, columns, and color depth output by cqcam
		// need to read in three numbers and skip any lines that start with a #
		read = 0;
		while (read < 3)
		{
			curchar = fgetc(fp);
			if ((char) curchar == '#')
			{ // skip this line
				while (fgetc(fp) != '\n')
					/* do nothing */;
			}
			else
			{
				ungetc(curchar, fp);
				ret = fscanf(fp, "%d", &(num[read]));
				read++;
			}
		}
		while (fgetc(fp) != '\n')
			/* pass the last newline character */;

		*cols = num[0];
		*rows = num[1];
		*colors = num[2];

		if (*cols > 0 && *rows > 0)
		{
			image = new rgb[(*rows) * (*cols)];

			if (image)
			{

				// Read the data
				if (binary)
					bytesRead = fread(image, sizeof(rgb), (*rows) * (*cols), fp);
				else
				{
					for (int x = 0; x < *rows; x++)
						for (int y = 0; y < *cols; y++)
						{
							int r, g, b;
							ret = fscanf(fp, "%d %d %d", &r, &g, &b);
							image[x + *rows * y] = rgb(r, g, b);
						}
				}

				fclose(fp);

				return image;
			}
		}

	}

	return (NULL);
} // end read_ppm

void ColorImage::load(const char * fname)
{
	using sven::rgb;

	if (data || dataYCrCb)
	{
		delete[] data;
//		delete[] dataYCrCb;
	}

	int c;
	rgb * colors = readPPM(fname, &w, &h, &c);

	N = w * h;
	printf("image loaded, w: %d, y: %d, c: %d.\n", w, h, c);

	data = new amy_lab[N];
	dataYCrCb = new amy_yCrCb[N];

	int i;
	for (i = 0; i < N; i++)
	{
		data[i] = amy_lab(colors[i]);
		dataYCrCb[i] = amy_yCrCb(colors[i]);
	}

	delete[] colors;
}

void GrayImage::save(const char *fname) const
{
	using sven::rgb;
	rgb * rval = new rgb[N];
	for (int i = 0; i < N; i++)
	{
		rval[i] = amy_lab(data[i], 0, 0).to_rgb();
	}

	//write out the grayscale image.
	FILE *fp;
	fp = fopen(fname, "wb");
	if (fp)
	{
		fprintf(fp, "P6\n");
		fprintf(fp, "%d %d\n%d\n", w, h, 255);
		fwrite(rval, sizeof(rgb), N, fp);
	}
	fclose(fp);

	delete[] rval;
}

void ColorImage::load_quant_data(const char *fname)
{
	using sven::rgb;

	int c;
	int i;
	rgb * colors = readPPM(fname, &w, &h, &c);

	N = w * h;
	printf("quantized image loaded, w: %d, y: %d, c: %d.\n", w, h, c);

	qdata.clear();

	using namespace std;

	map<rgb, int> q;
	map<rgb, int>::iterator r;
	for (i = 0; i < N; i++)
	{
		r = q.find(colors[i]);
		if (r == q.end())
			q[colors[i]] = 1;
		else
			r->second++;
	}

	printf("quantized image appears to use %d colors.\n", (int)q.size());
	qdata.resize(q.size());
	for (i = 0, r = q.begin(); r != q.end(); r++, i++)
	{
		qdata[i] = amy_lab_int(amy_lab(r->first), r->second);
	}

	//	data = new amy_lab[N];

	//	int i;
	//	for( i=0;i<N;i++) data[i]=amy_lab(colors[i]);

	delete[] colors;
}

void ColorImage::save(const char* fname, bool type) const
{
	using sven::rgb;
	rgb * rval = new rgb[N];
	if (!type)
	{
		for (int i = 0; i < N; i++)
		{
			rval[i] = amy_yCrCb(dataYCrCb[i].y, 0, 0).to_rgb();
		}
	}
	else
	{
		for (int i = 0; i < N; i++)
		{
			rval[i] = amy_yCrCb(dataYCrCb[i].y, dataYCrCb[i].Cr, dataYCrCb[i].Cr).to_rgb();
		}
	}

	//write out the grayscale image.
	FILE *fp;
	fp = fopen(fname, "wb");
	if (fp)
	{
		fprintf(fp, "P6\n");
		fprintf(fp, "%d %d\n%d\n", w, h, 255);
		fwrite(rval, sizeof(rgb), N, fp);
	}
	fclose(fp);

	delete[] rval;
}

void GrayImage::saveColor(const char *fname, const ColorImage &source) const
{
	using sven::rgb;
	rgb * rval = new rgb[N];
	printf("Saving Color2Gray + Chrominance\n");
	for (int i = 0; i < N; i++)
	{
		rval[i]
				= amy_lab(data[i], ((source.data)[i]).a, ((source.data)[i]).b).to_rgb();
	}

	//write out the grayscale image.
	FILE *fp;
	fp = fopen(fname, "wb");
	if (fp)
	{
		fprintf(fp, "P6\n");
		fprintf(fp, "%d %d\n%d\n", w, h, 255);
		fwrite(rval, sizeof(rgb), N, fp);
	}
	fclose(fp);

	delete[] rval;
}
