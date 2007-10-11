#ifndef __QEXC_HH__
#define __QEXC_HH__

#include <string>
#include <exception>
#include <errno.h>

namespace QEXC
{
    // This namespace contains some base classes on which an exception
    // handling system can be based.  It defines a basic framework and
    // a few convenience classes.

    // Note about throw() clauses in method declarations: they should
    // be avoided in almost all situations because they may thwart
    // applications' attempts to handle unexpected exeptions in a
    // clean way.  For an excellent discussion of this, see Scott
    // Meyers' "More Effective C++" book.  The thrust of the argument
    // is that overly restrictive throw() clauses cause unexpected
    // exceptions to terminate programs ungracefully even if
    // higher-level functions try to catch them.  If you are going to
    // use a throw() clause, there should be a very good reason, and
    // you should make sure that your function actually takes some
    // action to really avoid propagation of exceptions thrown by
    // functions it calls.  We use throw() clauses for our destructors
    // here because we consider having destructors of your exception
    // objects themeselves throwing exceptions to create a situation
    // from which recovery is nearly impossible.  Also, we derive from
    // std::exception, and follow its own model for declaring its
    // exception clauses.  You may wish to use throw() on destructors
    // of classes derived from these exceptions.  As a reminder, when
    // an exception is thrown that is not listed in a throw() clause,
    // the programs' registered "unexpected" handler is called.  The
    // unexpected handler may not return; it may only call
    // terminate(), exit(), or throw another exception.

    // The class hierarchy is as follows:

    //   std::exception
    //   |
    //   +-> QEXC::Base
    //       |
    //       +-> QEXC::General
    //       |
    //       +-> QEXC::Internal

    // QEXC::General is the base class of all standard user-defined
    // exceptions and "expected" error conditions raised by QClass.
    // Applications or libraries using QClass are encouraged to derive
    // their own exceptions from these classes if they wish.  It is
    // entirely reasonable for code to catch QEXC::General or specific
    // subclasses of it as part of normal error handling.

    // QEXC::Internal is reserved for internal errors.  These should
    // be used only for situations that indicate a likely bug in the
    // software itself.  This may include improper use of a library
    // function.  Operator errors should not be able to cause Internal
    // errors.  (There may be some exceptions to this such as users
    // invoking programs that were intended only to be invoked by
    // other programs.)  QEXC::Internal should generally not be
    // trapped except in terminate handlers or top-level exception
    // handlers which will want to translate them into error messages
    // and cause the program to exit.  Such top-level handlers may
    // want to catch std::exception instead.

    // Think of QEXC::Internal as analogous to Java's Error,
    // QEXC::General as Java's Exception, and QEXC::Base as Java's
    // Throwable.

    // All subclasses of QEXC::Base implement a const unparse() method
    // which returns a std::string const&.  They also override
    // std::exception::what() to return a char* with the same value.
    // unparse() should be implemented in such a way that a program
    // catching QEXC::Base or std::exception can use the text returned
    // by unparse() (or what()) without any exception-specific
    // adornment.  (The program may prefix the program name or other
    // general information.)  Note that std::exception::what() is a
    // const method that returns a const char* and throws no
    // exceptions.  For this reason, it is essential that unparse()
    // return a const reference to a string so that what() can be
    // implemented by calling unparse().  This means that the string
    // that unparse() returns a reference to must not be allocated on
    // the stack in the call to unparse().  The recommended way to do
    // this is for derived exception classes to store their string
    // descriptions by calling the protected setMessage() method and
    // then to not override unparse().

    class Base: public std::exception
    {
	// This is the common base class for all exceptions in qclass.
	// Application/library code should not generally catch this
	// directly.  See above for caveats.
      public:
	Base();
	Base(std::string const& message);
	virtual ~Base() throw() {}
	virtual std::string const& unparse() const;
	virtual const char* what() const throw();

      protected:
	void setMessage(std::string const& message);

      private:
	std::string message;
    };

    class General: public Base
    {
	// This is the base class for normal user/library-defined
	// error conditions.
      public:
	General();
	General(std::string const& message);
	virtual ~General() throw() {};
    };

    // Note that Internal is not derived from General.  Internal
    // errors are too severe.  We don't want internal errors
    // accidentally trapped as part of QEXC::General.  If you are
    // going to deal with internal errors, you have to do so
    // explicitly.
    class Internal: public Base
    {
      public:
	Internal(std::string const& message);
	virtual ~Internal() throw() {};
    };

    // Below are some predefined exceptions for very common cases.

    // The "System" exception wraps system error messages.  A common
    // pattern is to call a system or standard library call and
    // translate its error status to a System exception.
    class System: public General
    {
      public:
	System(std::string const& prefix, int sys_errno);
	virtual ~System() throw() {};
	int getErrno() const;

      private:
	int sys_errno;
    };

    // Convenience functions for wrapping fopen and for wrapping calls
    // that return -1 and set errno.  Wrapping such calls with these
    // routines will make them throw QEXC::System if they fail.

    // If status == -1, throw QEXC::System with the given description
    // and the current value of errno.  Usage:
    // errno_wrapper("calling xyz", xyz(whatever));
    int errno_wrapper(std::string const& description, int status)
	throw (QEXC::System);

    // If FILE is null, throw QEXC::System with the given error
    // status.  Usage:
    // FILE* f = fopen_wrapper("opening " + filename,
    //                         fopen(filename, "r"));
    FILE* fopen_wrapper(std::string const& description, FILE* file)
	throw (QEXC::System);

    // The "Timeout" exception is intended to be thrown by blocking
    // functions with timeouts.  Examples could include waiting on a
    // condition variable or waiting for I/O.
    class TimeOut: public General
    {
      public:
	TimeOut(std::string const& prefix);
	virtual ~TimeOut() throw() {};
    };
};

#endif // __QEXC_HH__
