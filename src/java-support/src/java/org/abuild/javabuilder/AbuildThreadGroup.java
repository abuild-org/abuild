package org.abuild.javabuilder;

class AbuildThreadGroup extends ThreadGroup
{
    String job = null;

    public AbuildThreadGroup()
    {
	super("AbuildThreadGroup");
    }

    public void setJob(String j)
    {
	this.job = j;
    }

    public String getJob()
    {
	return this.job;
    }
}
