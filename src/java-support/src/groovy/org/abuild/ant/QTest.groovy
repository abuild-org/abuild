package org.abuild.ant

import org.apache.tools.ant.Project
import org.apache.tools.ant.Task
import org.apache.tools.ant.BuildException
import org.abuild.groovy.Util

class QTest extends Task
{
    boolean stdoutIsTTY
    boolean emacsMode

    // VAR=val separated by \000
    String environment

    public void execute () throws BuildException
    {
	AntBuilder ant = new AntBuilder(project)

        def build = project.properties['basedir']
        def tty = this.stdoutIsTTY ? "1" : "0"
        def command = 'qtest-driver'
        def args = ['-datadir', '../qtest',
            '-bindirs', '..:.', '-covdir', '..',
            "-stdout-tty=${tty}"]
        if (Util.inWindows)
        {
            // Find qtest-driver in path and figure out how to invoke
            // it.  qtest requires cygwin perl, so it won't work
            // without cygwin in the path anyway.  Look for either
            // something that starts with #!/.../perl or #!/.../sh,
            // which could be a wrapper script.
            def pathSep = project.properties['path.separator']
            def path = System.env['PATH'].split(pathSep)
            String interpreter
            String driver
            for (p in path)
            {
                def candidate = new File(p, 'qtest-driver')
                if (candidate.isFile())
                {
                    candidate.withReader {
                        reader ->
                        def firstline = reader.readLine()
                        def m = (firstline =~ /#!(\S+)/)
                        if (m)
                        {
                            interpreter = new File(m.group(1)).name
                            if (interpreter == 'env')
                            {
                                m = firstline =~ /env (\S+)/
                                if (m)
                                {
                                    interpreter = m.group(1)
                                }
                            }
                            driver = candidate.absolutePath
                        }
                    }
                }
                if ((interpreter == 'perl') || (interpreter == 'sh'))
                {
                    break
                }
            }

            // Perl, especially cygwin perl, needs /, not \ in a path.
            driver = driver.replaceAll('\\\\', '/')
            if (interpreter)
            {
                command = interpreter
                args = [driver, *args]
            }
        }

        try
        {
            ant.exec('failonerror':'true', 'executable': command,
                     'dir': build)
            {
                if (! this.emacsMode)
                {
                    env('key':'QTEST_EXTRA_MARGIN', 'value':12)
                }
                if (this.environment)
                {
                    this.environment.split('\000').each {
                        def m = (it =~ /^(.+?)=(.*)$/)
                        if (m)
                        {
                            String key = m[0][1]
                            String val = m[0][2]
                            env('key': key, 'value': val)
                        }
                    }
                }
                args.each {
                    arg('value':it)
                }
            }
        }
        catch (BuildException e)
        {
            throw new BuildException(e.message, location)
        }
    }
}
