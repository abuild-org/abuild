package com.example;
import org.junit.Assert;

public class PotatoTest
{
    Potato first_potato;
    Potato second_potato;
    static Boolean fail = false;

    public static void main(String[] args)
    {
	if ((args.length > 0) && (args[0].equals("fail")))
	{
	    fail = true;
	}
	Potato p1 = new Potato(null);
	Potato p2 = new Potato("baked");
	System.out.println(p1);
	System.out.println(p2);
	if (org.junit.runner.JUnitCore.runClasses(
		PotatoTest.class).wasSuccessful())
	{
	    System.out.println("JUnit tests passed");
	}
	else
	{
	    System.out.println("JUnit tests failed");
	}
    }

    @org.junit.Test public void firstTest()
    {
	Potato p = new Potato("mashed");
	Assert.assertTrue(p.getPreparation() == "mashed");
    }

    @org.junit.Test public void secondTest()
    {
	Assert.assertTrue(first_potato.getPreparation() == "fried");
	Assert.assertTrue(second_potato.getPreparation() == "roasted");
	if (fail)
	{
	    Assert.assertTrue(false);
	}
    }

    @org.junit.Test(expected=IndexOutOfBoundsException.class)
    public void thirdTest()
    {
	new java.util.ArrayList<Object>().get(0);
    }

    @org.junit.Before public void setUp()
    {
	System.out.println("In junit setup");
	first_potato = new Potato("fried");
	second_potato = new Potato("roasted");
    }

    @org.junit.After public void cleanUp()
    {
	System.out.println("In junit cleanup with " + first_potato + " and " +
			   second_potato);
    }
}
