package com.example.basic;

import com.example.other.Other;

/**
 * This is a little documentation block.  It tells you everything you
 * need to know about this class, which is nothing.
 */
public class BasicLibrary
{
    private int n;

    /**
     * Public constructor
     *
     * @param n   An integer
     */
    public BasicLibrary(int n)
    {
	this.n = n;
    }

    /**
     * Private function.
     *
     * @param str   A string.
     */
    private void fpriv(String str)
    {
    }

    /**
     * Protected function.
     *
     * @param str   A string.
     */
    protected void fprot(String str, Other o)
    {
    }

    /**
     * Public function.
     */
    public void hello()
    {
	System.out.println("Hello.  This is BasicLibrary(" + n + ").");
	Other.hello();
    }
}
