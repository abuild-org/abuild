#ifndef __FILELOCATION_HH__
#define __FILELOCATION_HH__

#include <string>
#include <iostream>

class FileLocation
{
  public:
    FileLocation();
    FileLocation(std::string const& filename, int lineno, int colno);

    std::string getFilename() const;
    int getLineno() const;
    int getColno() const;

    // Writes "filename:lineno:colno: " with sensible handling for
    // when lineno and/or colno are zero.  Does nothing for a
    // FileLocation constructed with the default constructor.
    friend std::ostream& operator<<(std::ostream&, FileLocation const&);

    // A sensible ordering is defined so that FileLocation objects can
    // be map keys or stored in sets.
    bool operator==(FileLocation const&) const;
    bool operator<(FileLocation const&) const;

  private:
    std::string filename;
    int lineno;
    int colno;
};

#endif // __FILELOCATION_HH__
