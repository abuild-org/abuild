package com.example.consumers;

public interface Consumer
{
    public void register();
    public void consume(ProcessorInterface processor, int number);
}
