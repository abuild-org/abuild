package com.example.executable;

import com.example.helper.Helper;
import com.example.handlers.h3.H3;
import com.example.handlers.h4.H4;
import com.example.executable.entry.Entry;

public class Main
{
    static
    {
	new H3().register();
	new H4().register();
    }

    public static void main(String[] args)
    {
	Entry.runExecutable(new Helper(), args);
    }
}
