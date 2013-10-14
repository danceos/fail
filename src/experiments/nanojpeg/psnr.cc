#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

#include "psnr.hpp"

static void get_remaining_file_contents(std::ifstream& in, std::string& contents)
{
	std::streampos startpos = in.tellg();
	in.seekg(0, std::ios::end);
	contents.resize(in.tellg() - startpos);
	in.seekg(startpos, std::ios::beg);
	in.read(&contents[0], contents.size());
	in.close();
}

static inline double square(double x)
{
	return x*x;
}

static double image_rgb_mse(const unsigned char *f1, const unsigned char *f2, unsigned width, unsigned height, double max)
{
	double sum = 0.0;
	for (unsigned y = 0; y < height; ++y) {
		for (unsigned x = 0; x < width; ++x) {
			sum += square(f1[x*y*3+0] - f2[x*y*3+0]);
			sum += square(f1[x*y*3+1] - f2[x*y*3+1]);
			sum += square(f1[x*y*3+2] - f2[x*y*3+2]);
		}
	}
	return sum / (3 * height * width);
}

static double image_rgb_psnr(const unsigned char *f1, const unsigned char *f2, unsigned width, unsigned height, double max)
{
	double mse = image_rgb_mse(f1, f2, width, height, max);
	return 20.0 * log10(max / sqrt(mse));
}

double PSNR::calculate(const std::string& img)
{
	if (img.size() < refimg_width * refimg_height * 3) {
		std::cerr << "image too small" << std::endl;
		return -999;
	}
	return image_rgb_psnr((unsigned char *) &refimg[0], (unsigned char *) &img[0], refimg_width, refimg_height, refimg_max);
}

bool PSNR::load_refimage(char const *refimage_filename)
{
	std::ifstream fs(refimage_filename, std::ios::in | std::ios::binary);
	if (!fs) {
		return false;
	}

	std::string type;
	fs >> type;
	if (type != "P6") { return false; }

	fs >> refimg_width;
	fs >> refimg_height;
	fs >> refimg_max;

	if (refimg_max != 255) {
		std::cerr << "something is wrong" << std::endl;
		return false;
	}

	// one whitespace character
	fs.ignore();

	get_remaining_file_contents(fs, refimg);
	if (refimg.size() < refimg_width * refimg_height * 3) {
		std::cerr << "image too small" << std::endl;
		return false;
	}
	return true;
}
