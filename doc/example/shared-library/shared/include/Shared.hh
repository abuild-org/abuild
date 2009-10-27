#ifndef __SHARED_HH__
#define __SHARED_HH__

class
#ifdef _WIN32
__declspec(dllexport)
#endif
Shared
{
  public:
    static void hello();
};

#endif // __SHARED_HH__
