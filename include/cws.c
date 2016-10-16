#include "cws.h"

bool cws_running;

f32 cws_delta_time = 0.0f;
u32 cws_time_since_last_frame = 0;
f32 START_TIME;
static u32 time_cap = 0;

static u32 fps_counter = 0;
static u32 fps = 60;
static u32 time_since_last_cap;

bool cws_create(unsigned int screenW, unsigned int screenH, const char * title, bool vsync)
{
	time_since_last_cap = SDL_GetTicks();
	cws_running = false;
	cws_log_open("info.log");
	
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		cws_log("SDL_Init failed! %s", SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); 
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
    
	main_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenW, screenH, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if(main_window == NULL)
	{
		cws_log("SDL_CreateWindow failed! %s", SDL_GetError());
		return false;
	}
    
	main_gl_context = SDL_GL_CreateContext(main_window);
	glewExperimental = GL_TRUE; 
	glewInit();
    
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	
	SDL_GL_SetSwapInterval(vsync);
	glClearColor(0.3f, 0.5f, 1.0f, 1.0f);

	IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
	TTF_Init();

	cwsVideoInit();
	cwsSceneInit();
	cwsGuiInit();

	cws_running = true;
	cws_log("%s","Initialization successfull!");
	START_TIME = SDL_GetTicks();
	return true;
}

long CALLGRIND_WAIT = 0;
void cws_run()
{
	cws_time_since_last_frame = SDL_GetTicks() - time_cap;
	time_cap = SDL_GetTicks();

	cws_running = poll_events();
    cwsGuiUpdate();
    cwsSceneUpdate();

	if(SDL_GetTicks() - time_since_last_cap > 1000)
	{
		fps = fps_counter;
		fps_counter = 0;

		char buf[32];
		sprintf(buf, "FPS[%d]", fps);
		SDL_SetWindowTitle(main_window, buf);
		time_since_last_cap = SDL_GetTicks();
	}

	fps_counter++;
}

void cws_close()
{
	cwsVideoDestroy();
	cwsSceneDestroy();
	cwsGuiDestroy();
	cws_log_close();
	SDL_GL_DeleteContext(main_gl_context);
	SDL_DestroyWindow(main_window);
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}

