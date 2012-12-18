#include "SDL.h"
#include "SDL_opengl.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH       256
#define HEIGHT      256
#define SCALE         2
#define USE_RECT_TEX  0

#define GL_CHECK_ERROR(s)  do { if (glGetError() != GL_NO_ERROR) { fprintf(stderr, "%s: Error at line %i in %s\n", (s), __LINE__, __FILE__); abort(); } } while (0)

#define max(x,y)  (((x) > (y)) ? (x) : (y))
#define min(x,y)  (((x) < (y)) ? (x) : (y))


static const char grad3[][3] = {
	{ 1, 1, 0 }, { -1, 1, 0 }, { 1, -1, 0 }, { -1, -1, 0 },
	{ 1, 0, 1 }, { -1, 0, 1 }, { 1, 0, -1 }, { -1, 0, -1 },
	{ 0, 1, 1 }, { 0, -1, 1 }, { 0, 1, -1 }, { 0, -1, -1 }
};

static const unsigned char perm[] = {
	182, 232, 51, 15, 55, 119, 7, 107, 230, 227, 6, 34, 216, 61, 183, 36,
	40, 134, 74, 45, 157, 78, 81, 114, 145, 9, 209, 189, 147, 58, 126, 0,
	240, 169, 228, 235, 67, 198, 72, 64, 88, 98, 129, 194, 99, 71, 30, 127,
	18, 150, 155, 179, 132, 62, 116, 200, 251, 178, 32, 140, 130, 139, 250, 26,
	151, 203, 106, 123, 53, 255, 75, 254, 86, 234, 223, 19, 199, 244, 241, 1,
	172, 70, 24, 97, 196, 10, 90, 246, 252, 68, 84, 161, 236, 205, 80, 91,
	233, 225, 164, 217, 239, 220, 20, 46, 204, 35, 31, 175, 154, 17, 133, 117,
	73, 224, 125, 65, 77, 173, 3, 2, 242, 221, 120, 218, 56, 190, 166, 11,
	138, 208, 231, 50, 135, 109, 213, 187, 152, 201, 47, 168, 185, 186, 167, 165,
	102, 153, 156, 49, 202, 69, 195, 92, 21, 229, 63, 104, 197, 136, 148, 94,
	171, 93, 59, 149, 23, 144, 160, 57, 76, 141, 96, 158, 163, 219, 237, 113,
	206, 181, 112, 111, 191, 137, 207, 215, 13, 83, 238, 249, 100, 131, 118, 243,
	162, 248, 43, 66, 226, 27, 211, 95, 214, 105, 108, 101, 170, 128, 210, 87,
	38, 44, 174, 188, 176, 39, 14, 143, 159, 16, 124, 222, 33, 247, 37, 245,
	8, 4, 22, 82, 110, 180, 184, 12, 25, 5, 193, 41, 85, 177, 192, 253,
	79, 29, 115, 103, 142, 146, 52, 48, 89, 54, 121, 212, 122, 60, 28, 42,

	182, 232, 51, 15, 55, 119, 7, 107, 230, 227, 6, 34, 216, 61, 183, 36,
	40, 134, 74, 45, 157, 78, 81, 114, 145, 9, 209, 189, 147, 58, 126, 0,
	240, 169, 228, 235, 67, 198, 72, 64, 88, 98, 129, 194, 99, 71, 30, 127,
	18, 150, 155, 179, 132, 62, 116, 200, 251, 178, 32, 140, 130, 139, 250, 26,
	151, 203, 106, 123, 53, 255, 75, 254, 86, 234, 223, 19, 199, 244, 241, 1,
	172, 70, 24, 97, 196, 10, 90, 246, 252, 68, 84, 161, 236, 205, 80, 91,
	233, 225, 164, 217, 239, 220, 20, 46, 204, 35, 31, 175, 154, 17, 133, 117,
	73, 224, 125, 65, 77, 173, 3, 2, 242, 221, 120, 218, 56, 190, 166, 11,
	138, 208, 231, 50, 135, 109, 213, 187, 152, 201, 47, 168, 185, 186, 167, 165,
	102, 153, 156, 49, 202, 69, 195, 92, 21, 229, 63, 104, 197, 136, 148, 94,
	171, 93, 59, 149, 23, 144, 160, 57, 76, 141, 96, 158, 163, 219, 237, 113,
	206, 181, 112, 111, 191, 137, 207, 215, 13, 83, 238, 249, 100, 131, 118, 243,
	162, 248, 43, 66, 226, 27, 211, 95, 214, 105, 108, 101, 170, 128, 210, 87,
	38, 44, 174, 188, 176, 39, 14, 143, 159, 16, 124, 222, 33, 247, 37, 245,
	8, 4, 22, 82, 110, 180, 184, 12, 25, 5, 193, 41, 85, 177, 192, 253,
	79, 29, 115, 103, 142, 146, 52, 48, 89, 54, 121, 212, 122, 60, 28, 42
};


static float __attribute__ ((pure))
dot3(const char a[], float x, float y, float z)
{
	return a[0]*x + a[1]*y + a[2]*z;
}

static float __attribute__ ((const))
lerp(float a, float b, float t)
{
	return (1-t)*a + t*b;
}

static float __attribute__ ((const))
fade(float t)
{
	return t*t*t*(t*(t*6-15)+10);
}

#define FASTFLOOR(x)  (((x) >= 0) ? (int)(x) : (int)(x)-1)

static float __attribute__ ((pure))
perlin3d(float x, float y, float z)
{
	/* Find grid points */
	int gx = FASTFLOOR(x);
	int gy = FASTFLOOR(y);
	int gz = FASTFLOOR(z);

	/* Relative coords within grid cell */
	float rx = x - gx;
	float ry = y - gy;
	float rz = z - gz;

	/* Wrap cell coords */
	gx = gx & 255;
	gy = gy & 255;
	gz = gz & 255;

	/* Calculate gradient indices */
	unsigned int gi[8];
	for (int i = 0; i < 8; i++) gi[i] = perm[gx+((i>>2)&1)+perm[gy+((i>>1)&1)+perm[gz+(i&1)]]] % 12;

	/* Noise contribution from each corner */
	float n[8];
	for (int i = 0; i < 8; i++) n[i] = dot3(grad3[gi[i]], rx - ((i>>2)&1), ry - ((i>>1)&1), rz - (i&1));

	/* Fade curves */
	float u = fade(rx);
	float v = fade(ry);
	float w = fade(rz);

	/* Interpolate */
	float nx[4];
	for (int i = 0; i < 4; i++) nx[i] = lerp(n[i], n[4+i], u);

	float nxy[2];
	for (int i = 0; i < 2; i++) nxy[i] = lerp(nx[i], nx[2+i], v);

	return lerp(nxy[0], nxy[1], w);
}


static unsigned int __attribute__ ((const))
rgba_f_to_i(float r, float g, float b, float a)
{
	unsigned int cr = (unsigned int)(0xff*r) & 0xff;
	unsigned int cg = (unsigned int)(0xff*g) & 0xff;
	unsigned int cb = (unsigned int)(0xff*b) & 0xff;
	unsigned int ca = (unsigned int)(0xff*a) & 0xff;
	return (ca << 24) | (cb << 16) | (cg << 8) | cr;
}

/* Blue */
static const float color_grad[][5] = {
	{ 0.0, 0.0, 0.0, 1.0, 0.3 },
	{ 0.0, 0.0, 0.25, 1.0, 0.45 },
	{ 0.2, 0.13, 0.93, 1.0, 0.25 },
	{ 1.0, 1.0, 1.0, 1.0, 0.0 }
};

/* Red */
/*
static const float color_grad[][5] = {
	{ 1.0, 1.0, 1.0, 1.0, 0.3 },
	{ 0.95, 0.95, 0.0, 1.0, 0.25 },
	{ 0.75, 0.35, 0.0, 1.0, 0.45 },
	{ 0.0, 0.0, 0.0, 1.0, 0.0 }
};
*/

static unsigned int
rgba_map(float v)
{
	float r, g, b, a;

	int j = 1;
	float s = 0.0;
	while (1) {
		if (v < s+color_grad[j-1][4]) {
			r = lerp(color_grad[j-1][0], color_grad[j][0], (v-s)/color_grad[j-1][4]);
			g = lerp(color_grad[j-1][1], color_grad[j][1], (v-s)/color_grad[j-1][4]);
			b = lerp(color_grad[j-1][2], color_grad[j][2], (v-s)/color_grad[j-1][4]);
			a = lerp(color_grad[j-1][3], color_grad[j][3], (v-s)/color_grad[j-1][4]);
			break;
		}

		if (color_grad[j][4] == 0.0) {
			r = color_grad[j][0];
			g = color_grad[j][1];
			b = color_grad[j][2];
			a = color_grad[j][3];
			break;
		}

		s += color_grad[j-1][4];
		j += 1;
	}

	return rgba_f_to_i(r, g, b, a);
}

#define GRAD_WIDTH  256

unsigned int rgba_grad[GRAD_WIDTH];

static void
rgba_gradient()
{
	for (int i = 0; i < GRAD_WIDTH; i++) rgba_grad[i] = rgba_map((float)i/(GRAD_WIDTH-1));
}

static void
perlin_map_rgb(unsigned int dest[], const float src[])
{
	for (int i = 0; i < WIDTH*HEIGHT; i++) dest[i] = rgba_grad[FASTFLOOR(src[i]*(GRAD_WIDTH-1))];
}


static void
setup_sdl() 
{
	const SDL_VideoInfo* video;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	atexit(SDL_Quit);

	video = SDL_GetVideoInfo();
	if (!video) {
		fprintf(stderr, "Couldn't get video information: %s\n", SDL_GetError());
		exit(1);
	}

	/* Set the minimum requirements for the OpenGL window */
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	/* Note the SDL_DOUBLEBUF flag is not required to enable double 
	 * buffering when setting an OpenGL video mode. 
	 * Double buffering is enabled or disabled using the 
	 * SDL_GL_DOUBLEBUFFER attribute.
	 */
	if (SDL_SetVideoMode(SCALE*WIDTH, SCALE*HEIGHT+SCALE*50, video->vfmt->BitsPerPixel, SDL_OPENGL) == 0) {
		fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
		exit(1);
	}
}

static void
setup_opengl()
{
	glEnable(GL_TEXTURE_2D);
	GL_CHECK_ERROR("glEnable");

	if (USE_RECT_TEX) {
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		GL_CHECK_ERROR("glEnable");
	}

	glClearColor(0, 0, 0, 0);
	GL_CHECK_ERROR("glClearColor");

	glViewport(0, 0, SCALE*WIDTH, SCALE*HEIGHT+SCALE*50);
	GL_CHECK_ERROR("glViewport");

	glClear(GL_COLOR_BUFFER_BIT);
	GL_CHECK_ERROR("glClear");

	glMatrixMode(GL_PROJECTION);
	GL_CHECK_ERROR("glMatrixMode");
	glLoadIdentity();
	GL_CHECK_ERROR("glLoadIdentity");

	glOrtho(0, SCALE*WIDTH, SCALE*HEIGHT+SCALE*50, 0, -1, 1);
	GL_CHECK_ERROR("glOrtho");

	glMatrixMode(GL_MODELVIEW);
	GL_CHECK_ERROR("glMatrixMode");
	glLoadIdentity();
	GL_CHECK_ERROR("glLoadIdentity");
}

GLuint textures[2];

static void
repaint()
{
	/* Bind noise texture */
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	GL_CHECK_ERROR("glBindTextures");

	if (USE_RECT_TEX) {
		glBegin(GL_QUADS);
			glTexCoord2i(0, 0);
			glVertex2f(0, 0);

			glTexCoord2i(WIDTH, 0);
			glVertex2f(SCALE*WIDTH, 0);

			glTexCoord2i(WIDTH, HEIGHT);
			glVertex2f(SCALE*WIDTH, SCALE*HEIGHT);

			glTexCoord2i(0, HEIGHT);
			glVertex2f(0, SCALE*HEIGHT);
		glEnd();
	} else {
		glBegin(GL_QUADS);
			glTexCoord2i(0, 0);
			glVertex2f(0, 0);

			glTexCoord2i(1, 0);
			glVertex2f(SCALE*WIDTH, 0);

			glTexCoord2i(1, 1);
			glVertex2f(SCALE*WIDTH, SCALE*HEIGHT);

			glTexCoord2i(0, 1);
			glVertex2f(0, SCALE*HEIGHT);
		glEnd();
	}

	/* Bind histogram texture */
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	GL_CHECK_ERROR("glBindTextures");

	glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex2f(0, SCALE*HEIGHT);

		glTexCoord2i(1, 0);
		glVertex2f(SCALE*WIDTH, SCALE*HEIGHT);

		glTexCoord2i(1, 1);
		glVertex2f(SCALE*WIDTH, SCALE*HEIGHT+SCALE*50);

		glTexCoord2i(0, 1);
		glVertex2f(0, SCALE*HEIGHT+SCALE*50);
	glEnd();
}

static void
recalculate_noise(int type, float z)
{
	static float noise[WIDTH*HEIGHT];
	static unsigned int noise_tex[WIDTH*HEIGHT];
	static unsigned int histogram[WIDTH];
	static unsigned int histogram_tex[WIDTH*WIDTH];

	/* Reset histogram */
	unsigned int histogram_max = 0;
	memset(histogram, 0, sizeof(unsigned int)*WIDTH);

	/* Create noise texture */
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int index = y*WIDTH+x;
			switch (type) {
				case 0:
					noise[index] = 0;
					for (int l = 2; l < 4; l++) {
						noise[index] += fabs((1.0/l)*perlin3d(((float)(1 << l)*x)/HEIGHT, ((float)(1 << l)*y)/HEIGHT, z));
					}
					noise[index] = sinf(-1.0+(0.8*M_PI*y)/HEIGHT + noise[index]);
					noise[index] = 0.5*noise[index] + 0.5;
					/*noise[index] = perlin3d(((float)(1 << 1)*x)/HEIGHT, ((float)(1 << 1)*y)/HEIGHT, z);
					  noise[index] = 0.5*noise[index] + 0.5;*/
					break;
				case 1:
					noise[index] = perlin3d(((float)(1 << 7)*x)/HEIGHT, ((float)(1 << 6)*y)/HEIGHT, z);
					noise[index] = 0.5*noise[index] + 0.5;
					break;
				case 2:
					noise[index] = perlin3d(((float)(1 << 8)*x)/HEIGHT, ((float)(1 << 2)*y)/HEIGHT, z);
					noise[index] = 0.5*noise[index] + 0.5;
					break;
				case 3:
					noise[index] = perlin3d(((float)(1 << 4)*x)/HEIGHT, ((float)(1 << 4)*y)/HEIGHT, z);
					noise[index] = 0.5*noise[index] + 0.5;
					break;
				case 4:
					noise[index] = 0;
					for (int l = 1; l < 5; l++) {
						noise[index] += (1.0/(1 << l))*perlin3d(((float)(1 << l)*x)/HEIGHT, ((float)(1 << l)*y)/HEIGHT, z);
					}
					noise[index] = 0.5 * noise[index] * ((float)(1 << 4))/((float)(1 << 4)-1.0) + 0.5;
					break;
				case 5:
					noise[index] = 0;
					for (int l = 1; l < 5; l++) {
						noise[index] += (1.0/(1 << l))*perlin3d(((float)(1 << l)*x)/HEIGHT, ((float)(1 << (l*l))*y)/HEIGHT, z);
					}
					noise[index] = 0.5 * noise[index] * ((float)(1 << 4))/((float)(1 << 4)-1.0) + 0.5;
					break;
				case 6:
					noise[index] = 0;
					for (int l = 1; l < 5; l++) {
						noise[index] += fabs((1.0/(1 << l))*perlin3d(((float)(1 << l)*x)/HEIGHT, ((float)(1 << l)*y)/HEIGHT, z));
					}
					noise[index] = noise[index] * ((float)(1 << 4))/((float)(1 << 4)-1.0) + 0.25;
					break;
				case 7:
					noise[index] = 0;
					for (int l = 1; l < 5; l++) {
						noise[index] += fabs((1.0/(1 << l))*perlin3d(((float)(1 << ((l % 2) ? l*l : l))*x)/HEIGHT, ((float)(1 << ((l % 2) ? l : l*l))*y)/HEIGHT, z));
					}
					noise[index] = noise[index] * ((float)(1 << 4))/((float)(1 << 4)-1.0) + 0.25;
					break;
				case 8:
					noise[index] = 0;
					for (int l = 1; l < 5; l++) {
						noise[index] += fabs((1.0/(1 << l))*perlin3d(((float)(1 << l)*x)/HEIGHT, ((float)(1 << l)*y)/HEIGHT, z));
					}
					noise[index] = sinf(-1.0+(1.8*M_PI*y)/HEIGHT + noise[index]);
					noise[index] = 0.5*noise[index] + 0.5;
					break;
				case 9:
					noise[index] = 0.0;
					for (int l = 1; l < 5; l++) {
						noise[index] += sinf(((float)l*M_PI*y)/HEIGHT + ((float)(5-l)*M_PI*x)/WIDTH + fabs(perlin3d(((float)(1 << l)*x)/HEIGHT, ((float)(1 << l)*y)/HEIGHT, z)));
					}
					noise[index] = (1.0/8.0)*noise[index] + 0.5;
					break;
			}

			/* Update histogram */
			unsigned int histogram_index = FASTFLOOR(noise[index]*WIDTH);
			histogram_index = min(histogram_index, WIDTH-1);
			histogram[histogram_index] += 1;
			histogram_max = max(histogram[histogram_index], histogram_max);
		}
	}

	perlin_map_rgb(noise_tex, noise);

	/* Bind noise texture */
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	GL_CHECK_ERROR("glBindTextures");

	if (USE_RECT_TEX) {
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 4, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, noise_tex);
		GL_CHECK_ERROR("glTexImage2D");
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, 4, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, noise_tex);
		GL_CHECK_ERROR("glTexImage2D");
	}

	/* Create histogram texture */
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int index = y*WIDTH+x;
			if (histogram[x]/8.0 > HEIGHT-y) histogram_tex[index] = rgba_map((x)/(float)WIDTH);
			else histogram_tex[index] = rgba_f_to_i(0.03, 0.03, 0.03, 1.0);
		}
	}

	/* Bind histogram texture */
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	GL_CHECK_ERROR("glBindTextures");

	glTexImage2D(GL_TEXTURE_2D, 0, 4, WIDTH, WIDTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, histogram_tex);
}

#define FPS_LIMIT  25

static void
main_loop() 
{
	/* FPS */
	int fps = 0;
	int fps_ema = 0;
	int fps_ticks_delta = 0;
	int fps_ticks_delta_ema = 0;
	const float ema_alpha = 0.05;

	int type = 0;

	unsigned int t = 0;
	SDL_Event event;
	while (1) {
		/* process pending events */
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					exit(0);
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_SPACE) type = (type + 1) % 10;
					break;
			}
		}

		unsigned int before = SDL_GetTicks();

		/* update the screen */    
		recalculate_noise(type, (10.0*t)/512);
		repaint();
		SDL_GL_SwapBuffers();
		t += 1;

		/* FPS */
		unsigned int now = SDL_GetTicks();
		fps_ticks_delta = now - before;
		fps = 1000 / fps_ticks_delta;
		if (fps_ema) fps_ema = ema_alpha*fps + (1-ema_alpha)*fps_ema;
		else fps_ema = fps;
		if (fps_ticks_delta_ema) fps_ticks_delta_ema = ema_alpha*fps_ticks_delta + (1-ema_alpha)*fps_ticks_delta;
		else fps_ticks_delta_ema = fps_ticks_delta;

		if (t % 50 == 0) printf("fps: %i, fps_ema: %i, delta_ema: %i\n", fps, fps_ema, fps_ticks_delta_ema);

		unsigned int frame_time = 1000 / FPS_LIMIT;
		if (frame_time > fps_ticks_delta) SDL_Delay(frame_time - fps_ticks_delta);
	}
}

int
main(int argc, char* argv[])
{
	setup_sdl();   
	setup_opengl();

	rgba_gradient();

	/* Generate GL texture */
	glGenTextures(2, textures);
	GL_CHECK_ERROR("glGenTextures");

	/* Bind noise texture */
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	GL_CHECK_ERROR("glBindTextures");

	/* Set parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	GL_CHECK_ERROR("glTexParameteri");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GL_CHECK_ERROR("glTexParameteri");

	/* Bind histogram texture */
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	GL_CHECK_ERROR("glBindTextures");

	/* Set parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	GL_CHECK_ERROR("glTexParameteri");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GL_CHECK_ERROR("glTexParameteri");

	main_loop();

	return 0;
}
