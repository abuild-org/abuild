#include <iostream>

class Other
{
  public:
    Other();
};

static Other o;

Other::Other()
{
    std::cout << "Other::Other()" << std::endl;
}
