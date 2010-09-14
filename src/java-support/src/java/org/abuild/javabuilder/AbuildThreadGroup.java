package org.abuild.javabuilder;

class AbuildThreadGroup extends ThreadGroup
{
    int id = 0;
    String job = null;

    public AbuildThreadGroup(int id)
    {
	super("AbuildThreadGroup");
	this.id = id;
    }

    public int getID()
    {
	return this.id;
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
