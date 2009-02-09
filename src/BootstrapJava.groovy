#!/usr/bin/env groovy

import org.codehaus.groovy.control.CompilationFailedException

File srcDir = new File("java-support/src/java")
if (! srcDir.isDirectory())
{
    System.err.println "You must run this script from the abuild src directory"
    System.exit(2)
}
File buildDir = new File("java-support/abuild-java")
if (! buildDir.isDirectory())
{
    buildDir.mkdirs()
}

try
{
    Class.forName('new com.sun.tools.javac.Main')
}
catch (ClassNotFoundException e)
{
    def javaHome = new File(System.getProperty('java.home'))
    def toolsJar = new File("${javaHome.path}/lib/tools.jar")
    if ((! toolsJar.isFile()) && (javaHome.name == "jre"))
    {
        toolsJar = new File("${javaHome.parent}/lib/tools.jar")
    }
    if (toolsJar.isFile())
    {
        def loader = this.class.classLoader.rootLoader
        def path = "${toolsJar.absolutePath}"
        if (path =~ /^[^\/]/)
        {
            path = "/" + path.replace("\\", "/")
        }
        path = URLEncoder.encode(path, "UTF-8")
        loader.addURL("file://${path}".toURI().toURL())
    }
}

def ant = new AntBuilder()
def antJar = ant.project.getProperty('ant.core.lib')

ant.project.setBasedir(buildDir.absolutePath)
ant.taskdef('name': 'groovyc',
            'classname': 'org.codehaus.groovy.ant.Groovyc')

def distDir = 'dist'
def classesDir = 'classes'
ant.mkdir('dir': distDir)
ant.mkdir('dir': classesDir)

ant.javac('deprecation': 'yes',
          'destdir': classesDir,
          'classpath': antJar,
          'srcdir': srcDir.absolutePath)
ant.groovyc('destdir': classesDir,
            'srcdir': srcDir.absolutePath)
ant.jar('destfile': distDir + '/abuild-java-support.jar',
        'basedir': classesDir,
        'includes': '**/*.class')
