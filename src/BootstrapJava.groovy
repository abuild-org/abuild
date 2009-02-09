#!/usr/bin/env groovy

File srcDir = new File("java-support/src/java")
if (! srcDir.isDirectory())
{
    System.err.println "You must run this script from the abuild src directory"
    System.exit(2)
}
File buildDir = new File("java-support/abuild-java")
File destDir = new File(buildDir.path + "/dists")
if (! destDir.isDirectory())
{
    destDir.mkdirs()
}
// XXX Maybe we should use our embedded version of ant, if any, or
// maybe it's okay to require this for bootstrapping.
def antHome = System.getenv('ANT_HOME')
if (antHome == null)
{
    System.err.println "ANT_HOME must be set"
    System.exit(2)
}
def antJar = antHome + "/lib/ant.jar"
if (! new File(antJar).isFile())
{
    System.err.println "Can't find ant.jar in ${antJar}"
    System.exit(2)
}

def ant = new AntBuilder()
ant.project.setProperty('basedir', buildDir.absolutePath)
// Not sure why includeantruntime doesn't seem to work here...
ant.javac('deprecation': 'yes',
          'destdir': buildDir.path,
          'classpath': antJar,
          'srcdir': srcDir.path)
ant.jar('destfile': destDir.path + '/abuild-java-support.jar',
        'basedir': buildDir.path,
        'includes': '**/*.class')
