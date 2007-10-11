#ifndef __Z_HH__
#define __Z_HH__

class Z
{
  public:
#ifdef _WIN32
    __declspec(dllexport)
#endif
    static void hello();
};

#endif // __Z_HH__
