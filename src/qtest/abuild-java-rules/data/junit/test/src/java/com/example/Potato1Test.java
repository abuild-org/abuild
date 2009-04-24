package com.example;
import org.junit.Assert;

public class Potato1Test
{
    @org.junit.Test public void firstTest()
    {
	Potato p = new Potato("scalloped");
	Assert.assertTrue(p.getPreparation() == "scalloped");
    }
}
