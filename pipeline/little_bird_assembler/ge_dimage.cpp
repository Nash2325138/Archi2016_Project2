#include <cstdio>
#include <cstdlib>
void write_32bits_to_image(FILE *image, unsigned int number)
{
	for(int i=0 ; i<4 ; i++){
		unsigned char temp = (unsigned char)(number >> 24);
		fwrite(&temp, sizeof(unsigned char), 1, image);
		number <<= 8;
	}
}

int main(int argc, char const *argv[])
{
	FILE *dimage = fopen("dimage.bin", "wb");
	FILE *input = fopen("data.txt", "r");

	int temp;
	fscanf(input, "%d", &temp);
	write_32bits_to_image(dimage, (unsigned int)temp);
	fscanf(input, "%d", &temp);
	write_32bits_to_image(dimage, (unsigned int)temp);
	while(fscanf(input, "%d", &temp) > 0)
	{
		write_32bits_to_image(dimage, (unsigned int)temp);
	}
	return 0;
}