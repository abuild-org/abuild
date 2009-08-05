def basedir = properties['basedir']
def out = new File("${basedir}/.abuild-load.xml")
def files = []
// Plugins: load preplugin-ant.xml in each path
properties['abuild.plugins']?.split(/,\s*/).each {
    files << new File("${it}/preplugin-ant.xml")
}
// Since preplugin-ant.xml files are optional, filter out the ones
// that don't exist.
files = files.grep { it.isFile() }

// Create a project to import them
out.write('''<?xml version="1.0"?>
<project name="_load">
''')
files.each {
    out.append("<import file=\"${it.path}\"/>\n")
}
out.append("</project>\n")
