#ifndef __SHARED2_HH__
#define __SHARED2_HH__

class Shared2
{
  public:
#ifdef _WIN32
    __declspec(dllexport)
#endif
    static void hello();
};

#endif // __SHARED2_HH__
