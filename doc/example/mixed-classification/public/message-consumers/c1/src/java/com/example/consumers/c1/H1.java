package com.example.handlers.h1;

import com.example.handlers.HelperInterface;
import com.example.handlers.Handler;
import com.example.handlers.HandlerTable;

public class H1 implements Handler
{
    public void register()
    {
	HandlerTable.registerHandler(this);
    }

    public void handle(HelperInterface helper, int n)
    {
	System.out.println("public H1: " + helper.help(n));
    }
};
