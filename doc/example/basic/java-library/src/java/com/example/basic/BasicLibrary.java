package com.example.basic;

public class BasicLibrary
{
    private int n;

    public BasicLibrary(int n)
    {
	this.n = n;
    }

    public void hello()
    {
	System.out.println("Hello.  This is BasicLibrary(" + n + ").");
    }
}
