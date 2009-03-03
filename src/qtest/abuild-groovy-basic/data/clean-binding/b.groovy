try
{
    // Make sure variables added to the binding by previous scripts
    // are not visible here.
    println var
    assert false
}
catch (MissingPropertyException e)
{
    assert true
}

println "assertions passed"
