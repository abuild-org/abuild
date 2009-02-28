parameters {
    abuild {
        rules('java_proto')
    }
    java {
        jarName('java-program.jar')
        mainClass('com.example.basic.BasicProgram')
        wrapperName('java-program')
    }
}
