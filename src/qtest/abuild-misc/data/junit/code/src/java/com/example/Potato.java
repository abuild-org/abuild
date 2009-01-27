package com.example;

class Potato
{
    String preparation = "raw";

    public Potato(String preparation)
    {
	if (preparation != null)
	{
	    this.preparation = preparation;
	}
    }

    public String getPreparation()
    {
	return this.preparation;
    }

    public String toString()
    {
	return this.preparation + " potato";
    }
}
