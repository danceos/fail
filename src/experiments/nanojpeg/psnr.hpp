#ifndef __PSNR_HPP__
#define __PSNR_HPP__

#include <string>
#include <iostream>

class PSNR {
private:
	std::string refimg;
	unsigned refimg_width, refimg_height, refimg_max;

public:
	PSNR(char const *refimage_filename)
	{
		if (!load_refimage(refimage_filename)) {
			std::cerr << __func__ << " failed to load " << refimage_filename << std::endl;
		}
	}
	// we only accept P6 without comments
	bool load_refimage(char const *refimage_filename);
	double calculate(const std::string& img);
	unsigned getWidth() { return refimg_width; }
	unsigned getHeight() { return refimg_height; }
};

#endif
