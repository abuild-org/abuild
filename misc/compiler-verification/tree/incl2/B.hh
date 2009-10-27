#ifndef __B_HH__
#define __B_HH__

class B
{
  public:
#ifdef _WIN32
    __declspec(dllexport)
#endif
    static void hello();
};

#endif // __B_HH__
