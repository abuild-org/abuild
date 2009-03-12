package com.example;
import org.junit.Assert;

public class FailingTest
{
    @org.junit.Test public void someTest()
    {
	Assert.assertTrue(false);
    }
}
