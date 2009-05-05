// This code provides the "codegen" task, provided by this build item,
// to generate a class named by the user of the build item.

// Create a class to contain our targets.  From inside our class,
// properties in the script's binding are not available.  By doing our
// work inside a class, we are protected against a category of easy
// coding errors.  It doesn't matter if the class name collides with
// other classes defined in other rules.

class CodeGenerator
{
    def abuild
    def ant

    CodeGenerator(abuild, ant)
    {
        this.abuild = abuild
        this.ant = ant

        // Register the ant task.  The parameter
        // 'code-generator.classpath' is set in Abuild.interface.
        ant.taskdef('name': 'codegen',
                    'classname': 'com.example.codeGenerator.ExampleTask',
                    'classpath': abuild.resolve('code-generator.classpath'))
    }


    def codegenTarget()
    {
        // By using abuild.runActions, it is very easy for your custom
        // targets to support production of multiple artifacts.  This
        // method illustrates the usual pattern.

        // Create a map of default attributes and initialize this map
        // by initializing its members from the values of
        // user-supplied parameters.  In this case, the 'classname'
        // key gets a value that comes from the
        // 'codeGenerator.classname' parameter.  If the
        // codeGenerator.classname parameter is not set, the key will
        // exist in the map and will have a null value.
        def defaultAttrs = [
            'classname': abuild.resolveAsString('codeGenerator.classname')
        ]

        // Call abuild.runActions to do the work.  The first argument
        // is the name of a control parameter, the second argument is
        // a closure (here provided using Groovy's method closure
        // syntax), and the third argument is the default argument to
        // the closure.  If the control parameter is not initialized,
        // runActions will call the closure with the default
        // attributes.  Otherwise, the control parameter must contain
        // a list.  Each element of the list is either a map or a
        // closure.  If it is a map, any keys in defaultAttrs that are
        // not present in the map will be added to the map.  Then the
        // default closure will be called with the resulting map.  If
        // the element is a closure, the closure will be called, and
        // the default closure and attributes will be ignored.
        abuild.runActions('codeGenerator.codegen', this.&codegen, defaultAttrs)
    }

    def codegen(Map attributes)
    {
        // This is the method called by abuild.runActions as called
        // from codegenTarget when the user has not supplied his/her
        // own closure.  Since defaultAttrs contained the 'classname'
        // key, we know that it will always be present in the map,
        // even when the user supplied his/her own map.

        // In this case, we require classname to be set.  This means
        // the user must either have defined the
        // codeGenerator.classname parameter or provided the classname
        // key to the map.  If neither has been done, we fail.  In
        // some cases, it's more appropriate to just return without
        // doing anything, but in this case, the only reason a user
        // would select the codegenerator rules would be if they were
        // going to use this capability.  Also, in this example, we
        // ignore remaining keys in the attributes map, but in many
        // cases, it would be appropriate to remove the keys we use
        // explicitly and then pass the rest to whatever core ant task
        // is doing the heart of the work.

        def className = attributes['classname']
        if (! className)
        {
            ant.fail("property codeGenerator.classname must be defined")
        }
        ant.codegen('sourcedir': abuild.resolve('java.dir.generatedSrc'),
                    'classname': className)
    }
}

// Instantiate our class and add codegenTarget as a closure for the
// generate target.  We could also have added a custom target if we
// wanted to, but rather than cluttering things up with additional
// targets, we'll use the generate target which exists specifically
// for this purpose.

def codeGenerator = new CodeGenerator(abuild, ant)
abuild.addTargetClosure('generate', codeGenerator.&codegenTarget)
