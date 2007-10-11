#ifndef __PROJECTLIB_HPP__
#define __PROJECTLIB_HPP__

#include <CommonLib1.hpp>

class ProjectLib
{
  public:
    ProjectLib(int n = 5);
    void hello();

  private:
    ProjectLib(ProjectLib const&);
    ProjectLib& operator=(ProjectLib const&);

    CommonLib1 cl1;
};

#endif // __PROJECTLIB_HPP__
