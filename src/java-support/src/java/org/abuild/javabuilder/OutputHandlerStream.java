package org.abuild.javabuilder;

// The implementation choice of deriving this stream from
// ByteArrayOutputStream came from
// http://blogs.sun.com/nickstephen/entry/java_redirecting_system_out_and

import java.io.ByteArrayOutputStream;
import java.io.IOException;

class OutputHandlerStream extends ByteArrayOutputStream
{
    private OutputHandler handler;
    private boolean isError;

    public OutputHandlerStream(boolean isError, OutputHandler handler)
    {
        super();
	this.isError = isError;
        this.handler = handler;
    }

    public void flush() throws IOException
    {
        String record;
        synchronized(this) {
            super.flush();
            record = this.toString();
            super.reset();

	    try
	    {
		handler.sendOutput(this.isError, record);
	    }
	    catch (InterruptedException e)
	    {
		throw new IOException(e.toString());
	    }
	}
    }
}
