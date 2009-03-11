/*

 This build item finds the location of abuild's embedded groovy jar.
 This is okay for abuild's test suite, but it is not okay for general
 use for the following reasons

  - In general, a product should not depend on the presence of abuild
    at runtime

  - Although this specific code doesn't output a system-dependent
    path, any build item that does should have a native platform type.

*/

import org.abuild.groovy.Util

abuild.addTargetClosure('all') {
    File lib = new File(abuild.abuildTop + "/lib")
    def groovyJar = lib.list().grep { it ==~ /groovy.*\.jar/ }
    if (! ((groovyJar instanceof List) && (groovyJar.size() == 1)))
    {
        fail("unable to find groovyJar under ${lib.absolutePath}")
    }
    groovyJar = Util.absToRel(lib.absolutePath + '/' + groovyJar[0],
                              abuild.buildDirectory)
    File out = new File(abuild.buildDirectory.absolutePath + "/after.interface")
    out.write("abuild.classpath.external = ${groovyJar}\n")
}
