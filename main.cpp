/*
 * main.cpp
 *
 *  Created on: 18/07/2011
 *      Author: wendell & agnus
 */

#include "utility.h"

using namespace std;

int main(int argc, char **argv)
{
	locale loc = locale("");
	cout.imbue(loc);

	if (argc < 2)
	{
		cout << "Usage: C2G_GPU <image_name.ext> <alpha> <theta>" << endl;
		exit(1);
	}

	float alpha = atoi(argv[2]);
	float tetha = atof(argv[3]) * (M_PI / 180.);

	float ctetha = cosf(tetha), stetha = sinf(tetha);

	//Dados para warm-up da placa
	amy_lab dummyIn[16 * 16];
	float dummyOut[16 * 16];

	for (int i = 0; i < 16 * 16; ++i)
	{
		dummyIn[i].l = 1;
		dummyIn[i].a = 2;
		dummyIn[i].b = 3;
	}

	//Chamada de warm-up da placa
	cuColor2Gray(dummyIn, dummyOut, 16 * 16, 1, 1, 1);

	clock_t start = clock();

	ColorImage rgb;
	rgb.load(argv[1]);

	//	for (int i = 0; i < rgb.w * rgb.h; ++i) {
	//		rgb.data[i].l = 1;//(float)i;
	//		rgb.data[i].a = 2;
	//		rgb.data[i].b = 3;
	//	}
	GrayImage c2g(rgb);
	GrayImage deb(rgb);

	//Número de pixels da imagem

	clock_t startAlg = clock();

	unsigned int timer = 0;
	float cuTempo;
	cutCreateTimer(&timer);
	cutStartTimer(timer);

	//Chamar a função de configuração do kernel

	cuColor2Gray(rgb.data, c2g.data, c2g.N, alpha, ctetha, stetha);

	cuTempo = cutGetTimerValue(timer);
	cutStopTimer(timer);

	clock_t end = clock();

	cout << "\nc2g completed in " << (end - start) / (float) CLOCKS_PER_SEC
			<< " seconds OR " << fixed << cuTempo << " milliseconds." << endl;

	float sum = 0;
	for (int i = 0; i < rgb.w * rgb.h; ++i)
	{
		sum += rgb.data[i].l;
	}
	//	cout << "CPU sum: " << sum << endl;

	fstream metrFile;
	metrFile.imbue(loc);

	metrFile.open("metrica.csv", fstream::in);
	if (metrFile.fail())
	{
		metrFile.open("metrica.csv", fstream::out);
		metrFile << "NOME; " << "COLUNAS; " << "LINHAS; " << "ALPHA; "
				<< "THETA; " << "T LAB; " << "T C2G" << endl;
		metrFile.close();
	}
	else
	{
		metrFile.close();
	}
	metrFile.open("metrica.csv", fstream::out | fstream::app);
	metrFile << "\"" << (string) argv[1] << "\"" << "; " << rgb.w << "; "
			<< rgb.h << "; " << alpha << "; " << tetha * (180 / M_PI) << "; "
			<< (startAlg - start) / (float) CLOCKS_PER_SEC << "; " << fixed
			<< cuTempo << endl;
	metrFile.close();

	string parametros;

	std::ostringstream strs;
	strs.imbue(loc);
	strs << "alfa_" << alpha << "_tetha_" << tetha * (180 / M_PI) << "_";
	parametros = strs.str();

	string nameS = ((string) argv[1] + "_");
	string name;

	nameS.replace(nameS.end() - 5, nameS.end(), "_");
	nameS.replace(nameS.begin(), nameS.begin() + 5, "data/res/");
	name = nameS + "Gray.ppm";
	nameS += parametros;

	if (!fopen(name.c_str(), "r"))
		rgb.save(name.c_str(), false);
	name = nameS + "C2G.ppm";
	c2g.save(name.c_str());

	return 0;
}
