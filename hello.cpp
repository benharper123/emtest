#include <stdio.h>
#include <emscripten.h>
#include <stdint.h>
#include <math.h>

typedef double TColor;

TColor lab_xyz(TColor x)
{
	if (x > 0.206893034)
		return x * x * x;
	else
		return (x - 4.0 / 29.0) / 7.787037;
}

TColor xyz_rgb(TColor r)
{
	return 255 * (r <= 0.00304 ? 12.92 * r : 1.055 * pow(r, 1 / 2.4) - 0.055);
}

TColor clamp(TColor x, TColor _min, TColor _max)
{
	return x < _min ? _min : x > _max ? _max : x;
}

extern "C"
{

TColor EMSCRIPTEN_KEEPALIVE square(TColor x)
{
	return x * x;
}

void lch2rgb_internal(TColor l, TColor c, TColor h, uint8_t* rgb)
{
	// convert to Lab		
	TColor a = cos(h) * c;
	TColor b = sin(h) * c;

	// convert to rgb
	TColor X = 0.950470;
	TColor Y = 1.0;
	TColor Z = 1.088830;
	TColor y = (l + 16.0) / 116.0;
	TColor x = y + a / 500.0;
	TColor z = y - b / 200.0;
	x = lab_xyz(x) * X;
	y = lab_xyz(y) * Y;
	z = lab_xyz(z) * Z;
	rgb[0] = (uint8_t) clamp(xyz_rgb(3.2404542 * x - 1.5371385 * y - 0.4985314 * z), 0, 255);
	rgb[1] = (uint8_t) clamp(xyz_rgb(-0.9692660 * x + 1.8760108 * y + 0.0415560 * z), 0, 255);
	rgb[2] = (uint8_t) clamp(xyz_rgb(0.0556434 * x - 0.2040259 * y + 1.0572252 * z), 0, 255);

}

void EMSCRIPTEN_KEEPALIVE lch2rgb(double* lch, uint8_t* rgb)
{
	lch2rgb_internal((TColor) lch[0], (TColor) lch[1], (TColor) lch[2], rgb);
}

void EMSCRIPTEN_KEEPALIVE lch_colorwheel(double lum, int width, int height, uint8_t* px)
{
	int cx = width / 2;
	int cy = height / 2;
	TColor radius = cx;
	for (int y = 0; y < height; y++) {
		int ry = y - cy;
		uint8_t* line = px + y * width * 4;
		for (int x = 0; x < 512; x++, line += 4) {
			int rx = x - cx;
			TColor d = sqrt((TColor) (rx * rx + ry * ry));
			TColor h = atan2((TColor) ry, (TColor) rx);
			TColor c = (TColor) 100.0 * d / radius;
			uint8_t rgb[3];
			lch2rgb_internal(lum, c, h, rgb);
			line[0] = rgb[0];
			line[1] = rgb[1];
			line[2] = rgb[2];
			line[3] = 255;
		}
	}
}

}

//int main(int argc, char** argv)
//{
//	return 0;
//}