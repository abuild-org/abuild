#ifndef __SHARED1_HH__
#define __SHARED1_HH__

class Shared1
{
  public:
#ifdef _WIN32
    __declspec(dllexport)
#endif
    static void hello();
};

#endif // __SHARED1_HH__
