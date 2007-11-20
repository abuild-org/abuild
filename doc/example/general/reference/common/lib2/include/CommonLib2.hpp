#ifndef __COMMONLIB2_HPP__
#define __COMMONLIB2_HPP__

#include <CommonLib3.hpp>

class CommonLib2: public CommonLib3
{
  public:
    CommonLib2(int n);
    virtual ~CommonLib2();
    virtual void talkAbout();
};

#endif // __COMMONLIB2_HPP__
