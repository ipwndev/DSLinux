/*
 The following is only used when building DLLs under WIN32
*/

/* CET - FIXME - class definitions in headers still need to be fixed */

#if defined(WIN32) && defined(FLEK_SHARED)
#  ifdef FLEK_LIBRARY
#    define FLEK_API            __declspec(dllexport)
#  else
#    define FLEK_API            __declspec(dllimport)
#  endif
#  ifdef FLEK_UI_LIBRARY
#    define FLEK_UI_API         __declspec(dllexport)
#  else
#    define FLEK_UI_API         __declspec(dllimport)
#  endif
#  ifdef FLEK_GL_LIBRARY
#    define FLEK_GL_API         __declspec(dllexport)
#  else
#    define FLEK_GL_API         __declspec(dllimport)
#  endif
#  ifdef FLEK_XML_LIBRARY
#    define FLEK_XML_API        __declspec(dllexport)
#  else
#    define FLEK_XML_API        __declspec(dllimport)
#  endif
#  ifdef FLEK_JPEG_LIBRARY
#    define FLEK_JPEG_API       __declspec(dllexport)
#  else
#    define FLEK_JPEG_API       __declspec(dllimport)
#  endif
#else
#  define FLEK_API
#  define FLEK_UI_API
#  define FLEK_GL_API
#  define FLEK_XML_API
#  define FLEK_JPEG_API
#endif
