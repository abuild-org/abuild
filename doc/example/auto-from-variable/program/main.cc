#include <FileProvider.hh>
#include <FileProvider_file.hh>
#include <iostream>

int main()
{
    FileProvider fp;
    std::cout << "Showing contents of " << FILE_LOCATION << ":" << std::endl;
    fp.showFileContents();
    return 0;
}
