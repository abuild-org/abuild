package com.example.helper;

import com.example.handlers.HelperInterface;

public class Helper implements HelperInterface
{
    public String help(int n)
    {
	return "sensitive helper: n*n = " + n*n;
    }
}
