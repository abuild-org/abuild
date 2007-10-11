package com.example.handlers;

public interface Handler
{
    public void register();
    public void handle(HelperInterface helper, int number);
}
