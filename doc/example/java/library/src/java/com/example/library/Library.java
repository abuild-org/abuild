package com.example.library;

import com.example.library.generated.Negator;

public class Library
{
    private int value = 0;
    private Negator n = new Negator();

    public Library(int value)
    {
	this.value = value;
    }

    public int getOppose()
    {
	return n.negate(value);
    }
}
