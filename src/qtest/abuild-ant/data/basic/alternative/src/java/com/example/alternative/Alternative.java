package com.example.alternative;

import com.example.library.Library;

public class Alternative
{
    public static void main(String[] args)
    {
	int value = 12;
	Library lib = new Library(value);
	System.out.println("The opposite of " + value +
			   " is " + lib.getOppose());
    }
}
