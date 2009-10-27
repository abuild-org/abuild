#ifndef __SHARED_HH__
#define __SHARED_HH__

class Shared
{
  public:
#ifdef _WIN32
    __declspec(dllexport)
#endif
    static void hello();
};

#endif // __SHARED_HH__
