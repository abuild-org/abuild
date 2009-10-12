#ifndef __SHARED2_HH__
#define __SHARED2_HH__

class
#ifdef _WIN32
__declspec(dllexport)
#endif
Shared2
{
  public:
    static void hello();
};

#endif // __SHARED2_HH__
