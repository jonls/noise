#include "SDL.h"
#include "SDL_opengl.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH  256
#define HEIGHT 256
#define SCALE    2

#define GL_CHECK_ERROR(s)  do { if (glGetError() != GL_NO_ERROR) { fprintf(stderr, "%s: Error at line %i in %s\n", (s), __LINE__, __FILE__); abort(); } } while (0)




static const float grad3[][3] = {
	{ 1, 1, 0 }, { -1, 1, 0 }, { 1, -1, 0 }, { -1, -1, 0 },
	{ 1, 0, 1 }, { -1, 0, 1 }, { 1, 0, -1 }, { -1, 0, -1 },
	{ 0, 1, 1 }, { 0, -1, 1 }, { 0, 1, -1 }, { 0, -1, -1 }
};

static const unsigned int perm[] = {
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


static float
dot3(const float a[], float x, float y, float z)
{
	return a[0]*x + a[1]*y + a[2]*z;
}

static float
lerp(float a, float b, float t)
{
	return (1-t)*a + t*b;
}

#define FASTFLOOR(x)  (((x) >= 0) ? (int)(x) : (int)(x)-1)

static float
simplex3d(float x, float y, float z)
{
	/* Skew input space */
	float s = (x+y+z)*(1.0/3.0);
	int i = FASTFLOOR(x+s);
	int j = FASTFLOOR(y+s);
	int k = FASTFLOOR(z+s);

	/* Unskew */
	float t = (float)(i+j+k)*(1.0/6.0);
	float gx0 = i-t;
	float gy0 = j-t;
	float gz0 = k-t;
	float x0 = x-gx0;
	float y0 = y-gy0;
	float z0 = z-gz0;

	/* Determine simplex */
	int i1, j1, k1;
	int i2, j2, k2;

	if (x0 >= y0) {
		if (y0 >= z0) {
			i1 = 1; j1 = 0; k1 = 0;
			i2 = 1; j2 = 1; k2 = 0;
		} else if (x0 >= z0) {
			i1 = 1; j1 = 0; k1 = 0;
			i2 = 1; j2 = 0; k2 = 1;
		} else {
			i1 = 0; j1 = 0; k1 = 1;
			i2 = 1; j2 = 0; k2 = 1;
		}
	} else {
		if (y0 < z0) {
			i1 = 0; j1 = 0; k1 = 1;
			i2 = 0; j2 = 1; k2 = 1;
		} else if (x0 < z0) {
			i1 = 0; j1 = 1; k1 = 0;
			i2 = 0; j2 = 1; k2 = 1;
		} else {
			i1 = 0; j1 = 1; k1 = 0;
			i2 = 1; j2 = 1; k2 = 0;
		}
	}

	/* Calculate offsets in x,y,z coords */
	float x1 = x0 - i1 + (1.0/6.0);
	float y1 = y0 - j1 + (1.0/6.0);
	float z1 = z0 - k1 + (1.0/6.0);
	float x2 = x0 - i2 + 2.0*(1.0/6.0);
	float y2 = y0 - j2 + 2.0*(1.0/6.0);
	float z2 = z0 - k2 + 2.0*(1.0/6.0);
	float x3 = x0 - 1.0 + 3.0*(1.0/6.0);
	float y3 = y0 - 1.0 + 3.0*(1.0/6.0);
	float z3 = z0 - 1.0 + 3.0*(1.0/6.0);

	int ii = i % 256;
	int jj = j % 256;
	int kk = k % 256;

	/* Calculate gradient incides */
	int gi0 = perm[ii+perm[jj+perm[kk]]] % 12;
	int gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1]]] % 12;
	int gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2]]] % 12;
	int gi3 = perm[ii+1+perm[jj+1+perm[kk+1]]] % 12;

	/* Calculate contributions */
	float n0, n1, n2, n3;

	float t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
	if (t0 < 0) n0 = 0.0;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot3(grad3[gi0], x0, y0, z0);
	}

	float t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
	if (t1 < 0) n1 = 0.0;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot3(grad3[gi1], x1, y1, z1);
	}

	float t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
	if (t2 < 0) n2 = 0.0;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot3(grad3[gi2], x2, y2, z2);
	}

	float t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
	if (t3 < 0) n3 = 0.0;
	else {
		t3 *= t3;
		n3 = t3 * t3 * dot3(grad3[gi3], x3, y3, z3);
	}

	/* Return scaled sum of contributions */
	return 32.0*(n0 + n1 + n2 + n3);
}

static void
perlin_scale_bias(float dest[], float scale, float bias)
{
	for (int i = 0; i < WIDTH*HEIGHT; i++) {
		dest[i] = (dest[i] * scale) + bias;
	}
}

static float
perlin_max(const float src[])
{
	float max = -1/0.0;
	for (int i = 0; i < WIDTH*HEIGHT; i++) {
		if (src[i] > max) max = src[i];
	}
	return max;
}

static float
perlin_min(const float src[])
{
	float min = 1/0.0;
	for (int i = 0; i < WIDTH*HEIGHT; i++) {
		if (src[i] < min) min = src[i];
	}
	return min;
}


static unsigned int
rgba_f_to_i(float r, float g, float b, float a)
{
	unsigned int cr = (unsigned int)(0xff*r) & 0xff;
	unsigned int cg = (unsigned int)(0xff*g) & 0xff;
	unsigned int cb = (unsigned int)(0xff*b) & 0xff;
	unsigned int ca = (unsigned int)(0xff*a) & 0xff;
	return (ca << 24) | (cb << 16) | (cg << 8) | cr;
}

static void
perlin_map_rgb(unsigned int dest[], const float src[])
{
	for (int i = 0; i < WIDTH*HEIGHT; i++) {
		float r, g, b;
		float v = src[i];
		if (v < 0.3) {
			r = lerp(0.0, 0.0, (v-0.0)/0.3);
			g = lerp(0.0, 0.0, (v-0.0)/0.3);
			b = lerp(0.0, 0.25, (v-0.0)/0.3);
		} else if (v < 0.75) {
			r = lerp(0.0, 0.2, (v-0.3)/(0.75-0.3));
			g = lerp(0.0, 0.13, (v-0.3)/(0.75-0.3));
			b = lerp(0.25, 0.93, (v-0.3)/(0.75-0.3));
		} else {
			r = lerp(0.2, 1.0, (v-0.75)/(1.0-0.75));
			g = lerp(0.13, 1.0, (v-0.75)/(1.0-0.75));
			b = lerp(0.93, 1.0, (v-0.75)/(1.0-0.75));
		}
		dest[i] = rgba_f_to_i(r, g, b, 1.0);
	}
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
	if (SDL_SetVideoMode(SCALE*WIDTH, SCALE*HEIGHT, video->vfmt->BitsPerPixel, SDL_OPENGL) == 0) {
		fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
		exit(1);
	}
}

static void
setup_opengl()
{
	glEnable(GL_TEXTURE_2D);
	GL_CHECK_ERROR("glEnable");

	glClearColor(0, 0, 0, 0);
	GL_CHECK_ERROR("glClearColor");

	glViewport(0, 0, SCALE*WIDTH, SCALE*HEIGHT);
	GL_CHECK_ERROR("glViewport");

	glClear(GL_COLOR_BUFFER_BIT);
	GL_CHECK_ERROR("glClear");

	glMatrixMode(GL_PROJECTION);
	GL_CHECK_ERROR("glMatrixMode");
	glLoadIdentity();
	GL_CHECK_ERROR("glLoadIdentity");

	glOrtho(0, SCALE*WIDTH, SCALE*HEIGHT, 0, -1, 1);
	GL_CHECK_ERROR("glOrtho");

	glMatrixMode(GL_MODELVIEW);
	GL_CHECK_ERROR("glMatrixMode");
	glLoadIdentity();
	GL_CHECK_ERROR("glLoadIdentity");
}

GLuint texture;

static void
repaint()
{
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

static float data[WIDTH*HEIGHT];
static unsigned int rgb_data[WIDTH*HEIGHT];

static void
recalculate_noise(int type, float z)
{
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			switch (type) {
				case 0:
					data[y*WIDTH+x] = simplex3d(((float)(1 << 2)*x)/WIDTH, ((float)(1 << 2)*y)/HEIGHT, z);
					break;
				case 1:
					data[y*WIDTH+x] = 0;
					for (int l = 1; l < 5; l++) {
						data[y*WIDTH+x] += (1.0/(1 << l))*simplex3d(((float)(1 << l)*x)/WIDTH, ((float)(1 << l)*y)/HEIGHT, z);
					}
					break;
				case 2:
					data[y*WIDTH+x] = 0;
					for (int l = 1; l < 5; l++) {
						data[y*WIDTH+x] += fabs((1.0/(1 << l))*simplex3d(((float)(1 << l)*x)/WIDTH, ((float)(1 << l)*y)/HEIGHT, z));
					}
					break;
				case 3:
					data[y*WIDTH+x] = 0;
					for (int l = 1; l < 5; l++) {
						data[y*WIDTH+x] += fabs((1.0/(1 << l))*simplex3d(((float)(1 << l)*x)/WIDTH, ((float)(1 << l)*y)/HEIGHT, z));
					}
					data[y*WIDTH+x] = sinf(-1.0+(1.8*M_PI*y)/WIDTH + data[y*WIDTH+x]);
					break;
			}
		}
	}

	float mx = perlin_max(data);
	float mn = perlin_min(data);
	perlin_scale_bias(data, 1.0/(mx-mn), mn/(mn-mx));
	perlin_map_rgb(rgb_data, data);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgb_data);
	GL_CHECK_ERROR("glTexImage2D");
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
	const float ema_alpha = 0.25;

	int type = 0;

	unsigned int t;
	SDL_Event event;
	while (1) {
		/* process pending events */
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					exit(0);
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_SPACE) type = (type + 1) % 4;
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

	/* Generate GL texture */
	glGenTextures(1, &texture);
	GL_CHECK_ERROR("glGenTextures");

	/* Bind texture */
	glBindTexture(GL_TEXTURE_2D, texture);
	GL_CHECK_ERROR("glBindTextures");

	/* Set parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	GL_CHECK_ERROR("glTexParameteri");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GL_CHECK_ERROR("glTexParameteri");

	main_loop();

	return 0;
}
