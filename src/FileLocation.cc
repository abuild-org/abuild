#include <FileLocation.hh>

FileLocation::FileLocation() :
    lineno(0),
    colno(0)
{
}

FileLocation::FileLocation(std::string const& filename,
			   int lineno, int colno) :
    filename(filename),
    lineno(lineno),
    colno(colno)
{
}

std::string
FileLocation::getFilename() const
{
    return this->filename;
}

int
FileLocation::getLineno() const
{
    return this->lineno;
}

int
FileLocation::getColno() const
{
    return this->colno;
}

std::ostream&
operator<<(std::ostream& s, FileLocation const& fl)
{
    int lineno = fl.getLineno();
    int colno = fl.getColno();
    std::string filename = fl.getFilename();

    bool wrote = false;
    if (! filename.empty())
    {
	wrote = true;
	s << filename << ":";
    }
    if (lineno != 0)
    {
	wrote = true;
	s << lineno << ":";
	if (colno != 0)
	{
	    s << colno << ":";
	}
    }
    if (wrote)
    {
	s << " ";
    }
    return s;
}

bool
FileLocation::operator==(FileLocation const& rhs) const
{
    return ((this->filename == rhs.filename) &&
	    (this->lineno == rhs.lineno) &&
	    (this->colno == rhs.colno));
}

bool
FileLocation::operator<(FileLocation const& rhs) const
{
    if (this->filename < rhs.filename)
    {
	return true;
    }
    else if (this->filename > rhs.filename)
    {
	return false;
    }
    else if (this->lineno < rhs.lineno)
    {
	return true;
    }
    else if (this->lineno > rhs.lineno)
    {
	return false;
    }
    else
    {
	return (this->colno < rhs.colno);
    }
}
