package org.abuild.javabuilder;

import java.util.concurrent.ThreadFactory;

class AbuildThreadFactory implements ThreadFactory
{
    public Thread newThread(Runnable r)
    {
	return new Thread(new AbuildThreadGroup(), r);
    }
}
