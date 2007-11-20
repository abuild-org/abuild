#ifndef __COMMONLIB3_HPP__
#define __COMMONLIB3_HPP__

class CommonLib3
{
  public:
    CommonLib3(int n);
    virtual ~CommonLib3();
    void count();
    virtual void talkAbout();

  protected:
    int getN();

  private:
    CommonLib3(CommonLib3 const&);
    CommonLib3& operator=(CommonLib3 const&);

    int n;
};

#endif // __COMMONLIB3_HPP__
