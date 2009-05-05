parameters {
    java.jarName = 'example-library.jar'

    // Generate a Negator class using code-generator.  If we wanted to
    // create multiple classes, we could instead set
    // codeGenerator.codegen to a list of maps with each map
    // containing a classname key.  For an example of setting a
    // parameter to a list of maps, see ../executable/Abuild.groovy.
    codeGenerator.classname = 'com.example.library.generated.Negator'

    // Use both java and codegenerator rules.
    abuild.rules = ['java', 'codegenerator']
}
