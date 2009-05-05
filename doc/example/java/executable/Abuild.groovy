parameters {
    java.jarName = 'example-executable.jar'

    // Here we are going to generate multiple wrapper scripts.  We do
    // this by appending two different maps to the java.wrapper
    // parameter, each of which has a name key and a mainclass key.
    // There are many choices of syntax for doing this.  Here we use
    // Groovy's << operator to add something to a list.  We could also
    // have appended twice to java.wrapper in two separate statements,
    // or we could have explicitly assigned it to a list of maps.
    java.wrapper <<
        ['name': 'example',
         'mainclass' : 'com.example.executable.Executable'] <<
        ['name': 'other',
         'mainclass' : 'com.example.executable.Other']

    abuild.rules = 'java'
}
