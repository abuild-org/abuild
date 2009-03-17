#ifndef __COMPATLEVEL_HH__
#define __COMPATLEVEL_HH__

class CompatLevel
{
  public:
    enum level_e { cl_1_0, cl_1_1 };
    CompatLevel(level_e level) :
	level(level)
    {
    }

    void setLevel(level_e level)
    {
	this->level = level;
    }

    bool allow_1_0() const
    {
	return (this->level <= cl_1_0);
    }

    bool allow_1_1() const
    {
	return (this->level <= cl_1_1);
    }

  private:
    level_e level;
};

#endif // __COMPATLEVEL_HH__
