
// XXX We could easily provide a way for the user to specify
// environment variables to be passed to qtest once we come up with a
// decent strategy for passing information from Abuild.groovy to
// target implementations.

abuild.setTarget('test-only') {
    def src = abuild.sourceDirectory.path
    def qtest = new File("${src}/qtest")
    if (qtest.isDirectory())
    {
        def build = abuild.buildDirectory.path
        def tty = abuild.ifc['ABUILD_STDOUT_IS_TTY'] ? "1" : "0"
        def command = 'qtest-driver';
        def args = ['-datadir', '../qtest',
            '-bindirs', '..:.', '-covdir', '..',
            "-stdout-tty=${tty}"]
        if (System.getProperty('os.name') =~ /(?i:windows).*/)
        {
            command = 'perl'
            args = ['-S', 'qtest-driver', *args]
        }

        ant.exec('failonerror':'true', 'executable': command,
                 'dir': build)
        {
            if (! abuild.buildArgs.emacsMode)
            {
                env('key':'QTEST_EXTRA_MARGIN',
                    'value':12)
            }
            if (abuild.defines.containsKey('TESTS'))
            {
                env('key':'TESTS',
                    'value': abuild.defines['TESTS'])
            }
            args.each {
                arg('value':it)
            }
        }
    }
}
