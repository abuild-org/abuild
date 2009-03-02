package org.abuild.groovy

import org.codehaus.groovy.runtime.StackTraceUtils

class Util
{
    static void printStackTrace(Throwable e)
    {
        boolean inTestSuite = (System.getenv("IN_TESTSUITE") != null)
        if (inTestSuite)
        {
            System.err.println("--begin stack trace--")
        }
        StackTraceUtils.deepSanitize(e)
        e.printStackTrace(System.err)
        if (inTestSuite)
        {
            System.err.println("--end stack trace--")
        }
    }
}
