#ifndef __EXTERNALDATA_HH__
#define __EXTERNALDATA_HH__

#include <string>

class ExternalData
{
  public:
    ExternalData();
    ExternalData(std::string const& declared_path,
		 bool read_only);
    std::string const& getDeclaredPath() const;
    std::string const& getAbsolutePath() const;
    bool isReadOnly() const;
    void setAbsolutePath(std::string const& absolute_path);

  private:
    std::string declared_path;
    std::string absolute_path;
    bool read_only;
};

#endif // __EXTERNALDATA_HH__
