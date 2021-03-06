#pragma once
#include "ImageContainer.h"
class DBImage :
	public ImageContainer
{
public:
	DBImage(): ImageContainer(){}

	DBImage(string img_file,Mat descriptor, vector<KeyPoint> kps): ImageContainer(img_file,descriptor, kps){}

	DBImage(const DBImage &o): ImageContainer(o){}

	void makeMask(int maskType = ImageContainer::CIRCLE, int side_radius = 9, int side2 = 16);

	/**
	 * Intializes the orb object based on the parameters
	 *
	 * @param: the parm object containing the orb pramaters
	 */
	void initDescriptor();

	/**
	 * Calculates the orb descriptor of the image
	 */
	bool calcDescriptor();
	
	/**
	 * returns the size of the drawable image
	 *
	 * @return: the size of the displayable image
	 */
	Size imageSize();

	/**
	 * Determines if the ImageContainer has an image to display
	 */
	bool hasImage();

	bool loadImage(){return false;}

	void getMat(Mat &mat){ };
};