parameters {
    abuild.rules = 'java'
    java.compile << { ant.echo('This is a compile closure.') }
    java.compile << [ 'deprecation': 'off', 'includes': ['**/Other.java'] ]
    java.compile << [ 'excludes': ['**/Other*.java'] ]
    java.jarName = 'prog.jar'
    java.packageJar << [:]
    java.packageJar << ['jarname': 'other.jar',
                        'mainclass': 'com.example.other.Other',
                        'extramanifestkeys' : ['custom-key':'medeco']]
    java.mainClass = 'com.example.basic.BasicProgram'
    java.wrapper << ['name' : 'wrapper']
    java.wrapper << ['name' : 'other', 'mainclass' : 'com.example.other.Other']
}
