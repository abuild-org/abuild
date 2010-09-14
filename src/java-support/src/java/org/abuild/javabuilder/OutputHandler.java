package org.abuild.javabuilder;

interface OutputHandler
{
    public void sendOutput(boolean is_error, String data)
	throws InterruptedException;
}
