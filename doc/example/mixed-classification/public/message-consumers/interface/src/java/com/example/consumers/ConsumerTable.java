package com.example.consumers;

import java.util.Vector;

public class ConsumerTable
{
    static private Vector<Consumer> consumers = new Vector<Consumer>();

    static public void registerConsumer(Consumer h)
    {
	consumers.add(h);
    }

    static public Vector<Consumer> getConsumers()
    {
	return consumers;
    }
}
