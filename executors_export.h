
#ifndef EXECUTORS_EXPORT_H
#define EXECUTORS_EXPORT_H

#ifdef EXECUTORS_STATIC_DEFINE
#  define EXECUTORS_EXPORT
#  define EXECUTORS_NO_EXPORT
#else
#  ifndef EXECUTORS_EXPORT
#    ifdef executors_EXPORTS
        /* We are building this library */
#      define EXECUTORS_EXPORT 
#    else
        /* We are using this library */
#      define EXECUTORS_EXPORT 
#    endif
#  endif

#  ifndef EXECUTORS_NO_EXPORT
#    define EXECUTORS_NO_EXPORT 
#  endif
#endif

#ifndef EXECUTORS_DEPRECATED
#  define EXECUTORS_DEPRECATED __declspec(deprecated)
#endif

#ifndef EXECUTORS_DEPRECATED_EXPORT
#  define EXECUTORS_DEPRECATED_EXPORT EXECUTORS_EXPORT EXECUTORS_DEPRECATED
#endif

#ifndef EXECUTORS_DEPRECATED_NO_EXPORT
#  define EXECUTORS_DEPRECATED_NO_EXPORT EXECUTORS_NO_EXPORT EXECUTORS_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef EXECUTORS_NO_DEPRECATED
#    define EXECUTORS_NO_DEPRECATED
#  endif
#endif

#endif
