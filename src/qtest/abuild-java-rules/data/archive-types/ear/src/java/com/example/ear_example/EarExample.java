package com.example.ear_example;

import com.example.jar_example.JarExample;
import com.example.har_example.HarExample;

public class EarExample
{
    private JarExample j = new JarExample();
    private HarExample h = new HarExample();

    JarExample getJarExample()
    {
	return j;
    }

    HarExample getHarExample()
    {
	return h;
    }
}
