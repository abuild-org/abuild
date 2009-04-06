#ifndef __BUILDITEMATTRIBUTES_HH__
#define __BUILDITEMATTRIBUTES_HH__

class BuildItemAttributes
{
  public:
    BuildItemAttributes();

    void setGlobalTreeDep(bool);
    bool getGlobalTreeDep() const;
    void setGlobalPlugin(bool);
    bool getGlobalPlugin() const;

  private:
    bool global_treedep;
    bool global_plugin;
};

#endif // __BUILDITEMATTRIBUTES_HH__
