#ifndef __FILEPROVIDER_HH__
#define __FILEPROVIDER_HH__

class FileProvider
{
  public:
    FileProvider();
    void showFileContents() const;

  private:
    char const* const filename;
};

#endif // __FILEPROVIDER_HH__
