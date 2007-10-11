package com.example.handlers.h4;

import com.example.handlers.HelperInterface;
import com.example.handlers.Handler;
import com.example.handlers.HandlerTable;

public class H4 implements Handler
{
    public void register()
    {
	HandlerTable.registerHandler(this);
    }

    public void handle(HelperInterface helper, int n)
    {
	System.out.println("sensitive H4: " + helper.help(n));
    }
};
