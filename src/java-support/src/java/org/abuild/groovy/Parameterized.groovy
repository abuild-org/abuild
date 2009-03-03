package org.abuild.groovy

public interface Parameterized
{
    public void setParameter(String name, Object value)
    public void appendParameter(String name, Object value)
    public void deleteParameter(String name)
    public Object resolve(String name)
}
