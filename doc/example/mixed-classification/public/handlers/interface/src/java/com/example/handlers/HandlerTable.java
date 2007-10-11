package com.example.handlers;

import java.util.Vector;

public class HandlerTable
{
    static private Vector<Handler> handlers = new Vector<Handler>();

    static public void registerHandler(Handler h)
    {
	handlers.add(h);
    }

    static public Vector<Handler> getHandlers()
    {
	return handlers;
    }
}
