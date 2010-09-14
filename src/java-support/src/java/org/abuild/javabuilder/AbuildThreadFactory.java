package org.abuild.javabuilder;

import java.util.concurrent.ThreadFactory;

class AbuildThreadFactory implements ThreadFactory
{
    int next_id = 0;

    public Thread newThread(Runnable r)
    {
	int id = 0;
	synchronized (this)
	{
	    id = ++next_id;
	}
	return new Thread(new AbuildThreadGroup(id), r);
    }
}
