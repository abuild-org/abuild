#include <FileLocation.hh>
#include <sstream>

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

FileLocation::operator std::string() const
{
    std::ostringstream s;
    bool wrote = false;
    if (! this->filename.empty())
    {
	wrote = true;
	s << this->filename;
    }
    if (this->lineno != 0)
    {
	if (wrote)
	{
	    s << ":";
	}
	s << this->lineno;
	if (this->colno != 0)
	{
	    s << ":" << this->colno;
	}
    }
    return s.str();
}

std::ostream&
operator<<(std::ostream& s, FileLocation const& fl)
{
    s << std::string(fl);
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
