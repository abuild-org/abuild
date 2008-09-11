#include <FileProvider.hh>
#include <FileProvider_file.hh>
#include <fstream>
#include <iostream>
#include <stdlib.h>

FileProvider::FileProvider() :
    filename(FILE_LOCATION)
{
}

void
FileProvider::showFileContents() const
{
    std::ifstream in(this->filename);
    if (! in.is_open())
    {
	std::cerr << "Can't open file " << this->filename << std::endl;
	exit(2);
    }
    char c;
    while (in.get(c))
    {
	std::cout << c;
    }
}
