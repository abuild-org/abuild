package org.abuild.groovy

class BuildFailure extends Exception
{
    BuildFailure(String message)
    {
        super(message)
    }
}
