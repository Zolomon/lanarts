#ifndef FONT_H_
#define FONT_H_

#include <GL/glu.h>
#undef GL_GLEXT_VERSION
#include "GLImage.h"

#include <ft2build.h>
#include <freetype/freetype.h>

/*Stores everything needed to render a character glyph in opengl */
struct char_data {
	int w,h;
	int advance;
	int left;
	int move_up;
	unsigned char * data;
	GLimage img;

	char_data(char ch, FT_Face face);
	~char_data();
};

/*Holds all of the information related to any freetype font that we want to create.*/
struct font_data {
	char_data* data[128];
	float h;//Height of the font
};

/*Initialize a font of size 'h' from a font file.*/
void init_font(font_data* fd, const char * fname, unsigned int h);
void release_font(font_data* fd);


#endif /* FONT_H_ */
