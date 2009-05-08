import org.abuild.groovy.Util

abuild.addTargetDependencies('test-only', ['test-qtest'])
abuild.addTargetClosure('test-qtest') {
    def src = abuild.sourceDirectory.path
    def qtest = new File("${src}/qtest")
    if (qtest.isDirectory())
    {
        def build = abuild.buildDirectory.path
        def tty = abuild.interfaceVars['ABUILD_STDOUT_IS_TTY']
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
            def pathSep = ant.project.properties['path.separator']
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
            driver = driver.replaceAll('\\\\', '/');
            if (interpreter)
            {
                command = interpreter
                args = [driver, *args]
            }
        }

        ant.exec('failonerror':'true', 'executable': command,
                 'dir': build)
        {
            if (! abuild.buildArgs.emacsMode)
            {
                env('key':'QTEST_EXTRA_MARGIN',
                    'value':12)
            }
            def toExport = abuild.resolveAsList('qtest.export', [])
            ['TESTS', 'TC_SRCS'].each {
                if (abuild.resolve(it) != null)
                {
                    toExport << it
                }
            }
            if (toExport)
            {
                // Windows wants environment variables to be all
                // upper-case.
                toExport.each {
                    env('key': it.toUpperCase(),
                        'value': abuild.resolveAsList(it).join(' '))
                }
            }
            args.each {
                arg('value':it)
            }
        }
    }
}
