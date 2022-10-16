#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <include/Texture.hpp>

void Texture::loadBMP(const char* filename){

	FILE* file;
	size_t bytesRead;
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int width, height;
	unsigned int imageSize;
	unsigned char* data;

	std::cout << "loadBMP : " << filename << std::endl;

	file = fopen(filename,"rb");
	if(!file) {
        
        std::cout << "ERROR: can't open texture at " << filename << std::endl;
    	fclose(file);
        exit(EXIT_FAILURE);
    }

    bytesRead = fread(header,1,54,file);
	if(bytesRead != 54){ 
        
        std::cout << "ERROR: file is not a bitmap" << std::endl;
    	fclose(file);
        exit(EXIT_FAILURE);
    }

	if(header[0] != 'B' || header[1] != 'M'){

        std::cout << "ERROR: file is not a bitmap" << std::endl;
    	fclose(file);
        exit(EXIT_FAILURE);
    }

    if(*(int*)&(header[0x1E]) != 0) {

    	std::cout << "ERROR: file is not 24 bits per pixel" << std::endl;
    	fclose(file);
        exit(EXIT_FAILURE);
    }

	if(*(int*)&(header[0x1C]) != 24) {

    	std::cout << "ERROR: file is not 24 bits per pixel" << std::endl;
    	fclose(file);
        exit(EXIT_FAILURE);
    }

	dataPos = *(int*)&(header[0x0A]);
	if(dataPos == 0) dataPos = 54;
	
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	
	imageSize = *(int*)&(header[0x22]);
	if(imageSize == 0) imageSize = width * height * 3;

	data = new unsigned char[imageSize];

	bytesRead = fread(data,1,imageSize,file);
	if(bytesRead != imageSize) {

		std::cout << "ERROR: could not read the file correctly (" << bytesRead << "B/" << imageSize << "B)" << std::endl;
    	fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
	
    this->height = height;
    this->width = width;

	this->rgb.resize(imageSize);

	for(unsigned int i = 0; i < imageSize; i++) this->rgb[i] = data[i];

    delete data;
}