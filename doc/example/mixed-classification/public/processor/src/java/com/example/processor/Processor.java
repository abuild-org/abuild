package com.example.processor;

import com.example.consumers.ProcessorInterface;

public class Processor implements ProcessorInterface
{
    public String process(int n)
    {
	return "public processor: n = " + n;
    }
}
