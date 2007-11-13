package com.example.executable;

import com.example.processor.Processor;
import com.example.executable.entry.Entry;

public class Main
{
    public static void main(String[] args)
    {
	Entry.runExecutable(new Processor(), args);
    }
}
