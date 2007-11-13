package com.example.consumers.c1;

import com.example.consumers.ProcessorInterface;
import com.example.consumers.Consumer;
import com.example.consumers.ConsumerTable;

public class C1 implements Consumer
{
    public void register()
    {
	ConsumerTable.registerConsumer(this);
    }

    public void consume(ProcessorInterface processor, int n)
    {
	System.out.println("public C1: " + processor.process(n));
    }
}
