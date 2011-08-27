#ifndef AMY_COLORS_H
#define AMY_COLORS_H

#include <math.h>

namespace sven
{
struct rgb
{
	unsigned char r, g, b;
	rgb()
	{
	}
	rgb(unsigned char r, unsigned char g, unsigned char b) :
		r(r), g(g), b(b)
	{
	}
	bool operator<(const rgb &c) const
	{
		if (r < c.r)
			return true;
		if (r > c.r)
			return false;
		if (g < c.g)
			return true;
		if (g > c.g)
			return false;
		return b < c.b;
	}
};

const float one_third =
		0.33333333333333333333333333333333333333333333333333333333333333;
}

inline float clamp(float x, float x_min, float x_max)
{

	if (x < x_min)
		return (x_min);
	else if (x > x_max)
		return (x_max);
	else
		return (x);
}

struct amy_xyz
{

	float X, Y, Z;

	amy_xyz(float X, float Y, float Z) :
		X(X), Y(Y), Z(Z)
	{
	}

	amy_xyz(const sven::rgb &c)
	{
		float B = c.b / 255.;
		float G = c.g / 255.;
		float R = c.r / 255.;

		X = 0.412453f * R + 0.357580f * G + 0.180423f * B;
		Y = 0.212671f * R + 0.715160f * G + 0.072169f * B;
		Z = 0.019334f * R + 0.119193f * G + 0.950227f * B;

		const float Xn = 0.9513f;
		const float Yn = 1.000f;
		const float Zn = 1.0886f;

		X = X / Xn;
		Y = Y / Yn;
		Z = Z / Zn;
	}

	sven::rgb to_rgb()
	{

		//for RGB [0,1] etc.
		float R = 3.240479f * X + -1.537150f * Y + -0.498535f * Z;
		float G = -0.969256f * X + 1.875992f * Y + 0.041556f * Z;
		float B = 0.055648f * X + -0.204043f * Y + 1.057311f * Z;
		R = clamp(R, 0.0, 1.0);
		G = clamp(G, 0.0, 1.0);
		B = clamp(B, 0.0, 1.0);

		return sven::rgb(static_cast<unsigned char> (R * 255),
				static_cast<unsigned char> (G * 255),
				static_cast<unsigned char> (B * 255));
	}
};

struct amy_lab
{
	float l, a, b;

	amy_lab()
	{
	}
	amy_lab(float l, float a, float b) :
		l(l), a(a), b(b)
	{
	}

	amy_lab(const sven::rgb &c)
	{
		amy_xyz temp(c);
		*this = amy_lab(temp);
	}
	amy_lab(amy_xyz &c)
	{
		using sven::one_third;

		float X_third = pow(c.X, (float) one_third);
		float Y_third = pow(c.Y, (float) one_third);
		float Z_third = pow(c.Z, (float) one_third);

		if (c.Y > 0.008856)
			l = (116.0f * (Y_third)) - 16.0f;
		else
			l = 903.3f * c.Y;

		a = 500.0f * ((X_third) - (Y_third));
		b = 200.0f * ((Y_third) - (Z_third));
	}

	sven::rgb to_rgb()
	{

		float P = (l + 16.0) / 116.0;

		// Define as constants
		float Xn = 0.9513f;
		float Yn = 1.000f;
		float Zn = 1.0886f;

		float X = Xn * pow(P + (a / 500), 3);
		float Y = Yn * pow(P, 3);
		float Z = Zn * pow(P - (b / 200), 3);

		return amy_xyz(X, Y, Z).to_rgb();
	}

};

struct amy_yCrCb
{
	float y, Cr, Cb;

	amy_yCrCb()
	{
	}

	amy_yCrCb(float y, float Cr, float Cb) :
		y(y), Cr(Cr), Cb(Cb)
	{
	}

	amy_yCrCb(const sven::rgb &c)
	{
		float B = c.b / 255.;
		float G = c.g / 255.;
		float R = c.r / 255.;

		y = 0.299 * R + 0.587 * G + 0.114 * B;
		Cr = (R - y) * 0.713;
		Cb = (B - y) * 0.564;
	}

	//	amy_yCrCb(amy_xyz &c)
	//	{
	//
	//	}

	sven::rgb to_rgb()
	{
		float R = y + 1.403 * (Cr);
		float G = y - 0.344 * Cr - 0.714 * Cb;
		float B = y + 1.773 * Cb;

		return sven::rgb(static_cast<unsigned char> (R * 255),
				static_cast<unsigned char> (G * 255),
				static_cast<unsigned char> (B * 255));
	}

};

#endif
