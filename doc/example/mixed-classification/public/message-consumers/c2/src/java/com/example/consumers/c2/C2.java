package com.example.consumers.c2;

import com.example.consumers.ProcessorInterface;
import com.example.consumers.Consumer;
import com.example.consumers.ConsumerTable;

public class C2 implements Consumer
{
    public void register()
    {
	ConsumerTable.registerConsumer(this);
    }

    public void consume(ProcessorInterface processor, int n)
    {
	System.out.println("public C2: " + processor.process(n));
    }
}
