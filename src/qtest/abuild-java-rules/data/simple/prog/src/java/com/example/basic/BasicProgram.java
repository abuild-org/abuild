package com.example.basic;

import com.example.basic.BasicLibrary;

public class BasicProgram
{
    public static void main(String[] args)
    {
	BasicLibrary l = new BasicLibrary(10);
	l.hello();
	OtherThing.printClassName(l);
    }
};
