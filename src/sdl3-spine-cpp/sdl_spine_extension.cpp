
#include <SDL3/SDL_stdinc.h>

#include "sdl_spine_extension.h"


void* CSdlSpineExtension::_alloc(size_t size, SDL_UNUSED const char* file, SDL_UNUSED int line)
{
	return ::SDL_malloc(size);
}

void* CSdlSpineExtension::_calloc(size_t size, SDL_UNUSED const char* file, SDL_UNUSED int line)
{
	return ::SDL_calloc(1, size);
}

void* CSdlSpineExtension::_realloc(void* ptr, size_t size, SDL_UNUSED const char* file, SDL_UNUSED int line)
{
	return ::SDL_realloc(ptr, size);
}

void CSdlSpineExtension::_free(void* mem, SDL_UNUSED const char* file, SDL_UNUSED int line)
{
	::SDL_free(mem);
}

char* CSdlSpineExtension::_readFile(SDL_UNUSED const spine::String& path, SDL_UNUSED int* length)
{
	return nullptr;
}

spine::SpineExtension* spine::getDefaultExtension()
{
	return new CSdlSpineExtension();
}
