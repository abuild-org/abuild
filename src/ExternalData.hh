#ifndef __EXTERNALDATA_HH__
#define __EXTERNALDATA_HH__

#include <string>

class ExternalData
{
  public:
    ExternalData();
    ExternalData(std::string const& declared_path);
    std::string const& getDeclaredPath() const;
    std::string const& getAbsolutePath() const;
    void setAbsolutePath(std::string const& absolute_path);

  private:
    std::string declared_path;
    std::string absolute_path;
};

#endif // __EXTERNALDATA_HH__
