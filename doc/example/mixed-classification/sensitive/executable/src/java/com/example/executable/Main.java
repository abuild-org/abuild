package com.example.executable;

import com.example.processor.MessageProcessor;
import com.example.consumers.c3.C3;
import com.example.consumers.c4.C4;
import com.example.executable.entry.Entry;

public class Main
{
    static
    {
	new C3().register();
	new C4().register();
    }

    public static void main(String[] args)
    {
	Entry.runExecutable(new MessageProcessor(), args);
    }
}
