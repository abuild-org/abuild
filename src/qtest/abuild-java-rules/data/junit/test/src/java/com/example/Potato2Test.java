package com.example;
import org.junit.Assert;

public class Potato2Test
{
    @org.junit.Test public void firstTest()
    {
	Potato p = new Potato("au gratin");
	Assert.assertTrue(p.toString().equals("au gratin potato"));
    }
}
