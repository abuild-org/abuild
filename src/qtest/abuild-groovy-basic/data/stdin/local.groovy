abuild.configureTarget('test') {
    ant.java(classname: 'TestSystemIn', fork: 'true') {
        classpath {
            pathelement(location: abuild.resolve('testin-jar'))
        }
    }
}
