#pragma once

#if defined(_WIN32)
  #if defined(PMUC_EXPORTS)
    #define DLL_PMUC_EXPORT __declspec(dllexport)
  #else  // !BUILDING_DLL
    #define DLL_PMUC_EXPORT __declspec(dllimport)
  #endif  // BUILDING_DLL
#else // / UNIX /
  #define DLL_PMUC_EXPORT
#endif // 
