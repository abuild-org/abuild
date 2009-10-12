#ifndef __SHARED1_HH__
#define __SHARED1_HH__

class
#ifdef _WIN32
__declspec(dllexport)
#endif
Shared1
{
  public:
    static void hello();
};

#endif // __SHARED1_HH__
