package com.example.consumers.c3;

import com.example.consumers.ProcessorInterface;
import com.example.consumers.Consumer;
import com.example.consumers.ConsumerTable;

public class C3 implements Consumer
{
    public void register()
    {
	ConsumerTable.registerConsumer(this);
    }

    public void consume(ProcessorInterface processor, int n)
    {
	System.out.println("sensitive C3: " + processor.process(n));
    }
};
