#define NAMESPACE QFlow
#if defined(_WIN32) || defined(_WIN64)
    #define EXECUTORS_EXPORT __declspec(dllexport)
#else
    #define EXECUTORS_EXPORT
#endif


