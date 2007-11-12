package com.example.handlers.h3;

import com.example.handlers.HelperInterface;
import com.example.handlers.Handler;
import com.example.handlers.HandlerTable;

public class H3 implements Handler
{
    public void register()
    {
	HandlerTable.registerHandler(this);
    }

    public void handle(HelperInterface helper, int n)
    {
	System.out.println("sensitive H3: " + helper.help(n));
    }
};
