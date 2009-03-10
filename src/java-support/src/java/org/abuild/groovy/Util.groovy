package org.abuild.groovy

import org.codehaus.groovy.runtime.StackTraceUtils

class Util
{
    static boolean inWindows =
        (System.getProperty('os.name') =~ /(?i:windows).*/)

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

    static String absToRel(String path, String container)
    {
        absToRel(new File(path), new File(container))
    }

    static String absToRel(String path, File container)
    {
        absToRel(new File(path), container)
    }

    static String absToRel(File path, String container)
    {
        absToRel(path, new File(container))
    }

    static String absToRel(path)
    {
        absToRel(path, System.getProperty('user.dir'))
    }

    private static boolean pathComponentsMatch(String s1, String s2)
    {
        // Do a non-case-sensitive comparison on Windows
        if (inWindows)
        {
            return (s1.compareToIgnoreCase(s2) == 0)
        }
        else
        {
            return (s1 == s2);
        }
    }

    static String absToRel(File path, File container)
    {
        // This is a port of Util::absToRel from the C++ code.  It is
        // tested using the same test cases.

        if (inWindows)
        {
            if ((path.absolutePath =~ /^.:/) &&
                (container.absolutePath =~ /^.:/) &&
                (path.absolutePath[0].toLowerCase() !=
                 container.absolutePath[0].toLowerCase()))
            {
                // No way to construct a relative path to a file on a
                // different drive.
                return path.absolutePath
            }
        }

        def sep = File.separator
        if (sep == '\\')
        {
            sep = '\\\\'
        }
        List pitems = path.absolutePath.split(sep)
        List litems = container.absolutePath.split(sep)

        while (pitems && litems && (pathComponentsMatch(pitems[0], litems[0])))
        {
            pitems.remove(0)
            litems.remove(0)
        }
        def result = new StringBuffer()
        result << '../' * litems.size() << pitems.join('/')
        if (! result)
        {
            result = '.'
        }
        else if (result =~ '/$')
        {
            result.setLength(result.length() - 1)
        }

        result
    }
}
