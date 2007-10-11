#ifndef __COMMONLIB1_HPP__
#define __COMMONLIB1_HPP__

class CommonLib1
{
  public:
    CommonLib1(int n);
    void countBackwards();

  private:
    CommonLib1(CommonLib1 const&);
    CommonLib1& operator=(CommonLib1 const&);

    int n;
};

#endif // __COMMONLIB1_HPP__
